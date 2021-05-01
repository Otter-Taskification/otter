#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include <macros/debug.h>
#include <macros/general.h>

#include <otter-ompt-header.h>
#include <otter-common.h>
#include <otter-datatypes/graph.h>
#include <otter-task-graph/task-graph.h>

static void destroy_graph_node_data(
    void *node_data, graph_node_type_t node_type);

/* flag which output format to use for task graph */
enum {
    format_dot,
    format_edge
};

/* The task graph is a graph with a few helper variables, including a lock so
   that multiple threads can add nodes/subgraphs to a single global task graph
*/
typedef struct task_graph_t {
    bool                 initialised;
    graph_t             *g;
    pthread_mutex_t      lock;
    char                 graph_output[TASK_GRAPH_BUFFSZ + 1];
    char                 graph_nodeattr[TASK_GRAPH_BUFFSZ + 1];
    int                  graph_output_format;
} task_graph_t;

/* A single static instance of a task graph exists within the program. Access is
   made thread-safe through a mutex.
*/
static task_graph_t Graph = {
    .initialised         = false,
    .g                   = NULL,
    .lock                = PTHREAD_MUTEX_INITIALIZER,
    .graph_output        = {0},
    .graph_nodeattr      = {0},
    .graph_output_format = format_dot
};

/* Initialise the task graph. Should be called once at program start.
   Aborts execution if initialisation fails for any reason.
   Returns true on success. Does nothing if the tree is aleady initialised.
   This function is NOT thread-safe as it it intended to be called before
   any threads are created.
 */
bool
task_graph_init(otter_opt_t *opt)
{
    if (Graph.initialised)
    {
        LOG_WARN("already initialised");
        return Graph.initialised;
    }

    LOG_INFO("initialising");

    if ((Graph.g = graph_create()) == NULL)
    {
        LOG_ERROR("failed to initialise, aborting");
        abort();
    }

    /* detect options set in environment */
    if ((opt->graph_output == NULL) || STR_EQUAL(opt->graph_output, ""))
        opt->graph_output = TASK_GRAPH_DEFAULT_GRAPH_NAME;
    if ((opt->graph_nodeattr == NULL) || STR_EQUAL(opt->graph_nodeattr, ""))
        opt->graph_nodeattr = TASK_GRAPH_DEFAULT_GRAPH_ATTR_NAME;

    if (opt->graph_format == NULL) opt->graph_format = "not set";

    char *ext = ".gv";
    if (STR_EQUAL(opt->graph_format, "edge")) {
        Graph.graph_output_format = format_edge;
        ext = ".csv";
    } else {
        Graph.graph_output_format = format_dot;
        ext = ".gv";
    }

    char *pos = NULL;
    char pid_str[PID_STR_SZ + 1] = {0};
    snprintf(pid_str, PID_STR_SZ, "%d", getpid());

    /* Copy graph output filename, extension, hostname, PID */
    pos = &Graph.graph_output[0]; 
    strncpy(Graph.graph_output, opt->graph_output, TASK_GRAPH_BUFFSZ); 
    pos = &Graph.graph_output[0] + strlen(Graph.graph_output); 
    strncpy(pos, ext, TASK_GRAPH_BUFFSZ - strlen(Graph.graph_output)); 
    pos = &Graph.graph_output[0] + strlen(Graph.graph_output); 
    if (opt->append_hostname)
    { 
        strcpy(pos, "."); 
        strncpy(pos+1, opt->hostname,
            TASK_GRAPH_BUFFSZ - strlen(Graph.graph_output)); 
        pos = &Graph.graph_output[0] + strlen(Graph.graph_output); 
    }
    strcpy(pos, ".");
    strncpy(pos+1, pid_str, TASK_GRAPH_BUFFSZ - strlen(Graph.graph_output));

    /* Copy graph output filename hostname, PID */
    pos = &Graph.graph_nodeattr[0]; 
    strncpy(Graph.graph_nodeattr, opt->graph_nodeattr, TASK_GRAPH_BUFFSZ);
    pos = &Graph.graph_nodeattr[0] + strlen(Graph.graph_nodeattr); 
    if (opt->append_hostname)
    { 
        strcpy(pos, "."); 
        strncpy(pos+1, opt->hostname,
            TASK_GRAPH_BUFFSZ - strlen(Graph.graph_nodeattr)); 
        pos = &Graph.graph_nodeattr[0] + strlen(Graph.graph_nodeattr); 
    }
    strcpy(pos, ".");
    strncpy(pos+1, pid_str, TASK_GRAPH_BUFFSZ - strlen(Graph.graph_nodeattr));

    return Graph.initialised = true;
}

void
task_graph_destroy(graph_free_node_data_t free_node_data)
{
    if (Graph.initialised == false)
    {
        LOG_WARN("task tree not initialised");
        return;
    }
    LOG_INFO("destroying");
    graph_destroy(Graph.g, free_node_data);
    pthread_mutex_destroy(&Graph.lock);
    Graph.initialised = false;
    return;    
}

/* add a node to the graph and return a reference to it - only for reference */
task_graph_node_t *
task_graph_add_node(
    task_graph_node_type_t node_type,
    task_graph_node_data_t node_data)
{
    /* if tree not initialised when trying to add a node, abort execution */
    if (false == Graph.initialised)
    {
        LOG_ERROR("task graph not initialised before adding node, aborting");
        abort();
    }

    /* lock the graph to ensure atomic access */
    pthread_mutex_lock(&Graph.lock);

    graph_node_t *node = graph_add_node(Graph.g, node_type,
        (graph_node_data_t) {.ptr = node_data.ptr});

    pthread_mutex_unlock(&Graph.lock);

    LOG_ERROR_IF((NULL == node), "failed to create node for task graph");

    #if DEBUG_LEVEL >= 4
    graph_print(Graph.g);
    #endif

    return (task_graph_node_t*) node;

}

/* declare an edge from src to dest */
void
task_graph_add_edge(
    task_graph_node_t *src,
    task_graph_node_t *dest)
{
    /* if graph not initialised when trying to add edge, abort execution */
    if (false == Graph.initialised)
    {
        LOG_ERROR("task graph not initialised before adding node, aborting");
        abort();
    }

    /* lock the graph to ensure atomic access */
    pthread_mutex_lock(&Graph.lock);

    graph_edge_t *edge = graph_add_edge(Graph.g,
        (graph_node_t*) src, (graph_node_t*) dest);

    pthread_mutex_unlock(&Graph.lock);

    LOG_ERROR_IF((edge == NULL), "failed to create edge for task graph");

    #if DEBUG_LEVEL >= 4
    graph_print(Graph.g);
    #endif

    return;    
}

/* move the nodes and edges from the subgraph into the task graph. does not
   create any edges between the two graphs. after this operation subgraph will
   be empty
*/
bool
task_graph_attach_subgraph(graph_t *subgraph)
{
    /* if graph not initialised when trying to add subgraph, abort execution */
    if (false == Graph.initialised)
    {
        LOG_ERROR("task graph not initialised before adding subgraph, aborting");
        abort();
    }

    /* lock the graph to ensure atomic access */
    pthread_mutex_lock(&Graph.lock);
    bool result = graph_union(Graph.g, subgraph);
    pthread_mutex_unlock(&Graph.lock);
    return result;
}

bool
task_graph_write(void)
{
    FILE *taskgraph = fopen(Graph.graph_output, "w");
    FILE *nodeattr  = fopen(Graph.graph_nodeattr, "w");
    LOG_INFO("output: \"%s\"", Graph.graph_output);
    LOG_INFO("output: \"%s\"", Graph.graph_nodeattr);

    if (taskgraph == NULL)
    {
        LOG_ERROR("failed to create file \"%s\": %s",
            Graph.graph_output, strerror(errno));
        errno = 0;
        if (nodeattr) fclose(nodeattr);
        return false;
    }

    if (nodeattr == NULL)
    {
        LOG_ERROR("failed to create file \"%s\": %s",
            Graph.graph_nodeattr, strerror(errno));
        errno = 0;
        if (taskgraph) fclose(taskgraph);
        return false;
    }

    /* Write file headers */
    switch (Graph.graph_output_format)
    {
        case format_dot:
            fprintf(taskgraph,
                "digraph \"\" {\n"
                "    graph   [fontname=\"helvetica\"];\n"
                "    node    [fontname=\"helvetica\" "
                    "shape=circle color=red style=solid"
                "];\n"
                "    label = \"\";\n"
                "\n"
            );
            break;
        case format_edge:
            fprintf(taskgraph, "%s,%s\n", "parent_task_id", "child_task_id");
            break;
        default:
            LOG_ERROR("invalid output format identifier: %d",
                Graph.graph_output_format);
            fclose(taskgraph);
            fclose(nodeattr);
            return false;
    }

    size_t num_nodes, num_edges, k;
    void *next = NULL;
    graph_get_num_nodes_edges(Graph.g, &num_nodes, &num_edges);

    /* For a dot-file, declare all nodes before writing edges */
    if (Graph.graph_output_format == format_dot)
    {
        uint64_t graph_node_id;
        graph_node_type_t graph_node_type;
        unique_id_t *otter_id = NULL;
        for (k=0; k<num_nodes; k++)
        {
            graph_scan_nodes(Graph.g,
                &graph_node_id, &graph_node_type,
                (graph_node_data_t*) &otter_id, &next);
            fprintf(taskgraph, "  %lu [node_type=%d otter_id=%lu]\n",
                graph_node_id, graph_node_type, *otter_id);
        }
    }

    /* Write edges */
    uint64_t src_id, dest_id;
    next = NULL;
    switch (Graph.graph_output_format)
    {
        case format_dot:
            for (k=0; k<num_edges; k++)
            {
                graph_scan_edges(Graph.g, &src_id, NULL, NULL,
                    &dest_id, NULL, NULL, &next);
                fprintf(taskgraph, "%lu -> %lu\n", src_id, dest_id);
            }
            break;
        case format_edge:
            for (k=0; k<num_edges; k++)
            {
                graph_scan_edges(Graph.g, &src_id, NULL, NULL,
                    &dest_id, NULL, NULL, &next);
                fprintf(taskgraph, "%lu,%lu\n", src_id, dest_id);
            }
            break;
        default:
            // pass
            break;
    }

    /* Write file footer */
    switch (Graph.graph_output_format)
    {
        case format_dot:
            fprintf(taskgraph, "\n}\n");
            break;
        default:
            // pass
            break;
    }
    return true;
}