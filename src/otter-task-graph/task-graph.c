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

#include <otter-ompt-header.h>
#include <otter-common.h>
#include <otter-core/otter.h>
#include <otter-datatypes/graph.h>
#include <otter-task-graph/task-graph.h>

static void destroy_node_data(
    void *node_data, graph_node_type_t node_type);

#define NODE_STYLE_STR_MAXLEN 64
#define NODE_ATTR_STR_MAXLEN  1024
#define NODE_DATA_STR_MAXLEN  1024
static char *task_graph_node_style(task_graph_node_type_t node_type);
static char *task_graph_node_label(
    task_graph_node_type_t node_type, void *node_data);
static char *task_graph_node_attr(
    uint64_t node_id, task_graph_node_type_t node_type, void *node_data);
static char *task_graph_node_data_repr(
    task_graph_node_type_t node_type, void *node_data);

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
    graph_node_data_t node_data)
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

    fprintf(nodeattr, "{ \"nodes\":  [\n");

    switch (Graph.graph_output_format)
    {
        case format_dot:
            fprintf(taskgraph,
                "digraph \"\" {\n"
                "    graph   [fontname=\"Source Code Pro\"];\n"
                "    node    [fontname=\"Source Code Pro\" "
                        "shape=circle color=red style=\"rounded,filled\""
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

    /* Write node attributes. For a dot-file, also declare all nodes before 
       writing edges */
    uint64_t node_id;
    graph_node_type_t node_type;
    void *node_data = NULL;
    for (k=0; k<num_nodes; k++)
    {
        graph_scan_nodes(Graph.g, &node_id, &node_type, (graph_node_data_t*) &node_data, &next);
        if (Graph.graph_output_format == format_dot)
        {
            fprintf(taskgraph, "  %lu [node_type=%d %s %s]\n",
                node_id, node_type,
                task_graph_node_style(node_type),
                task_graph_node_label(node_type, node_data)
            );
        }
        fprintf(nodeattr, "%s%s",
            task_graph_node_attr(node_id, node_type, node_data),
            k < num_nodes-1 ? ",\n" : ""
        );
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

    fprintf(nodeattr, "]  \n}\n");

    switch (Graph.graph_output_format)
    {
        case format_dot:
            fprintf(taskgraph, "\n}\n");
            break;
        default:
            // pass
            break;
    }
    fclose(taskgraph);
    fclose(nodeattr);
    return true;
}

static char *
task_graph_node_style(task_graph_node_type_t node_type)
{
    static char node_style_str[NODE_STYLE_STR_MAXLEN + 1] = {0};
    static const char *fmt_string = "shape=%s color=%s";

    char *shape =
        node_type == node_task_initial  ?               "record" :
        node_type == node_task_implicit ?               "record" :
        node_type == node_task_explicit ?               "record" :
        node_type == node_task_target   ?               "record" :
        (node_type == node_scope_parallel_begin) 
            || (node_type == node_scope_parallel_end) ? "parallelogram" :
        (node_type == node_scope_loop_begin) 
            || (node_type == node_scope_loop_end) ?     "diamond" :
        (node_type == node_scope_taskloop_begin) 
            || (node_type == node_scope_taskloop_end) ? "diamond" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier ?                "hexagon" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_implicit ?       "hexagon" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_explicit ?       "hexagon" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_implementation ? "hexagon" :
        (node_type & ~SCOPE_END_BIT) == node_sync_taskwait ?               "hexagon" :
        (node_type == node_scope_sync_taskgroup_begin) 
            || (node_type == node_scope_sync_taskgroup_end) ? "hexagon" :
        (node_type == node_scope_single_begin)
            || (node_type == node_scope_single_end) ? "triangle" :
                                                        "circle";

    char *color = 
        node_type == node_task_initial  ?               "yellow" :
        node_type == node_task_implicit ?               "blue" :
        node_type == node_task_explicit ?               "red" :
        node_type == node_task_target   ?               "purple" :
        (node_type == node_scope_parallel_begin) 
            || (node_type == node_scope_parallel_end) ? "green" :
        (node_type == node_scope_loop_begin) 
            || (node_type == node_scope_loop_end) ?     "orange" :
        (node_type == node_scope_taskloop_begin) 
            || (node_type == node_scope_taskloop_end) ? "cyan" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier ?                "red" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_implicit ?       "blue" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_explicit ?       "magenta" :
        (node_type & ~SCOPE_END_BIT) == node_sync_barrier_implementation ? "green" :
        (node_type & ~SCOPE_END_BIT) == node_sync_taskwait ?               "cyan" :
        (node_type == node_scope_sync_taskgroup_begin) 
            || (node_type == node_scope_sync_taskgroup_end) ? "grey" :
        (node_type == node_scope_single_begin)
            || (node_type == node_scope_single_end) ? "orange" :
                                                        "white";

    snprintf(&node_style_str[0], NODE_STYLE_STR_MAXLEN,
        fmt_string, shape, color);
    return &node_style_str[0];
}

static char *
task_graph_node_label(
    task_graph_node_type_t  node_type,
    void                   *node_data)
{
    static char node_label_str[NODE_ATTR_STR_MAXLEN + 1] = {0};
    const char *fmt_string = NULL;
    switch (node_type & ~SCOPE_END_BIT)
    {
        case node_scope_parallel_begin:
        {
            parallel_data_t *parallel_data = (parallel_data_t*) node_data;
            fmt_string = "label=\"%s parallel %lu\"";
            snprintf(node_label_str, NODE_ATTR_STR_MAXLEN, fmt_string,
                node_type & SCOPE_END_BIT ? "end" : "begin",
                parallel_data->id
            );
            break;
        }
        case node_scope_sections_begin:
        case node_scope_single_begin:
        case node_scope_loop_begin:
        case node_scope_taskloop_begin:
        {
            int scope_type = node_type & ~SCOPE_END_BIT;
            fmt_string = "label=\"%s %s\"";
            snprintf(node_label_str, NODE_DATA_STR_MAXLEN, fmt_string,
                node_type & SCOPE_END_BIT ? "end" : "begin",
                scope_type == node_scope_sections_begin ? "sections"  :
                scope_type == node_scope_single_begin   ? "single"    :
                scope_type == node_scope_loop_begin     ? "loop"      :
                scope_type == node_scope_taskloop_begin ? "taskloop"  :
                    "unknown scope"
            );
            break;
        }
        case node_task_initial:
        case node_task_implicit:
        case node_task_explicit:
        case node_task_target:
        {
            task_data_t *task_data = (task_data_t*) node_data;
            ompt_task_flag_t type = task_data->type;
            int flags = task_data->flags;
            fmt_string = "label=\"%s task %lu\"";
            snprintf(node_label_str, NODE_DATA_STR_MAXLEN, fmt_string,
                type == ompt_task_initial ? "initial" :
                    type == ompt_task_implicit ? "implicit" :
                    type == ompt_task_explicit ? "explicit" :
                    type == ompt_task_target ? "target" : "unknown",
                task_data->id
            );
            break;
        }
        case node_scope_sync_taskgroup_begin:
        {
            fmt_string = "label=\"%s taskgroup\"";
            snprintf(node_label_str, NODE_DATA_STR_MAXLEN, fmt_string,
                node_type & SCOPE_END_BIT ? "end" : "begin");
            break;
        }
        case node_sync_barrier:
        case node_sync_barrier_implicit:
        case node_sync_barrier_explicit:
        case node_sync_barrier_implementation:
        case node_sync_taskwait:
        case node_sync_reduction:
        {
            int sync_type = node_type & ~SCOPE_END_BIT;
            fmt_string = "label=\"%s\"";
            snprintf(node_label_str, NODE_DATA_STR_MAXLEN, fmt_string,
                sync_type == node_sync_barrier ? "barrier" :
                    sync_type == node_sync_barrier_implicit ? "implicit barrier" :
                    sync_type == node_sync_barrier_explicit ? "explicit barrier" :
                    sync_type == node_sync_barrier_implementation ? "implementation barrier" :
                    sync_type == node_sync_taskwait ? "taskwait" :
                    sync_type == node_scope_sync_taskgroup_begin ? "taskgroup" :
                    sync_type == node_scope_sync_taskgroup_end ? "taskgroup" :
                    sync_type == node_sync_reduction ? "reduction" : "unknown barrier"
            );
            break;
        }
        default:
            return "label=\"null\"";
    }
    return node_label_str;
}

static char *task_graph_node_attr(
    uint64_t                 node_id,
    task_graph_node_type_t   node_type,
    void                    *node_data)
{
    static char node_attr_str[NODE_ATTR_STR_MAXLEN + 1] = {0};
    static const char *fmt_string =
        "  {\n" 
        "    \"id\":         %lu,\n"
        "    \"type\":       %d,\n"
        "    \"endpoint\":   %s,\n"
        "    \"data\":       %s\n"
        "  }"
    ;
    snprintf(node_attr_str, NODE_ATTR_STR_MAXLEN, fmt_string,
        node_id,
        node_type & ~SCOPE_END_BIT,
        node_type & SCOPE_END_BIT ? "\"end\"" : "\"begin\"",
        task_graph_node_data_repr(node_type, node_data)
    );
    return &node_attr_str[0];
}

static char *
task_graph_node_data_repr(
    task_graph_node_type_t  node_type,
    void                   *node_data)
{
    static char node_data_repr[NODE_DATA_STR_MAXLEN + 1] = {0};
    const char *fmt_string = NULL;
    switch (node_type & ~SCOPE_END_BIT)
    {
        case node_scope_parallel_begin:
        case node_scope_parallel_end:
        {
            parallel_data_t *parallel_data = (parallel_data_t*) node_data;
            fmt_string = "\n"
                "      {\n"
                "        \"otter_id\":             %lu,\n"
                "        \"actual_parallelism\":   %d,\n"
                "        \"is_league\":            %s \n"
                "      }\n";
            snprintf(node_data_repr, NODE_DATA_STR_MAXLEN, fmt_string,
                parallel_data->id,
                parallel_data->actual_parallelism,
                parallel_data->flags & ompt_parallel_league ? 
                    "true" : "false"
            );
            break;
        }
        case node_task_initial:
        case node_task_implicit:
        case node_task_explicit:
        case node_task_target:
        {
            task_data_t *task_data = (task_data_t*) node_data;
            ompt_task_flag_t type = task_data->type;
            int flags = task_data->flags;
            fmt_string = "\n"
                "      {\n"
                "        \"otter_id\":             %lu,\n"
                "        \"task_type\":            \"%s\",\n"
                "        \"undeferred\":           %s,\n"
                "        \"untied\":               %s,\n"
                "        \"final\":                %s,\n"
                "        \"mergeable\":            %s,\n"
                "        \"merged\":               %s\n"
                "      }\n";
            snprintf(node_data_repr, NODE_DATA_STR_MAXLEN, fmt_string,
                task_data->id,
                type == ompt_task_initial ? "initial" :
                    type == ompt_task_implicit ? "implicit" :
                    type == ompt_task_explicit ? "explicit" :
                    type == ompt_task_target ? "target" : "unknown",
                flags & ompt_task_undeferred ? "true" : "false",
                flags & ompt_task_untied     ? "true" : "false",
                flags & ompt_task_final      ? "true" : "false",
                flags & ompt_task_mergeable  ? "true" : "false",
                flags & ompt_task_merged     ? "true" : "false"
            );
            break;
        }
        case node_sync_barrier:
        case node_sync_barrier_implicit:
        case node_sync_barrier_explicit:
        case node_sync_barrier_implementation:
        case node_sync_taskwait:
        case node_scope_sync_taskgroup_begin:
        case node_scope_sync_taskgroup_end:
        case node_sync_reduction:
        {
            int sync_type = node_type & ~SCOPE_END_BIT;
            fmt_string = "\n"
                "      {\n"
                "        \"sync_type\":     \"%s\"\n"
                "        \"sync_endpoint\": \"%s\"\n"
                "      }\n";
            snprintf(node_data_repr, NODE_DATA_STR_MAXLEN, fmt_string,
                sync_type == node_sync_barrier ? "barrier" :
                    sync_type == node_sync_barrier_implicit ? "implicit barrier" :
                    sync_type == node_sync_barrier_explicit ? "explicit barrier" :
                    sync_type == node_sync_barrier_implementation ? "implementation barrier" :
                    sync_type == node_sync_taskwait ? "taskwait" :
                    sync_type == node_scope_sync_taskgroup_begin ? "taskgroup" :
                    sync_type == node_scope_sync_taskgroup_end ? "taskgroup" :
                    sync_type == node_sync_reduction ? "reduction" : "unknown",
                node_type == node_scope_sync_taskgroup_begin ? "begin" : "end"
            );
            break;
        }
        default:
            return "null";
    }
    return node_data_repr;
}
