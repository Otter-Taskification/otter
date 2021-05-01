#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <otter-task-graph/task-tree.h>

#include <otter-datatypes/queue.h>
#include <otter-datatypes/dynamic-array.h>

#include <macros/debug.h>
#include <macros/general.h>

static bool tree_write_dot(FILE *taskgraph);
static void tree_write_dot_child_fmt(FILE *taskgraph, tree_node_id_t child_id);
static void tree_write_child_node_attributes(
    FILE *nodeattr, tree_node_id_t child_id);
static void tree_destroy_node(void *ptr);

/* A node is a container that belongs to a particular task. It maintains an
   array of the IDs of the child tasks spawned by the task
*/
struct tree_node_t {
    tree_node_id_t      parent_id;
    array_t            *children;
};

/* flag which output format to use for task tree */
enum {
    format_dot,
    format_edge,
    format_adjacency
};

/* A task tree is implemented as a queue of tree nodes plus a root node. Each
   item in the queue is a tree node, which contains the ID of the owning task 
   and a dynamic array of the task's children.

   The root node can be optionally used tomaintain a list of tasks with no 
   parent.
 */
typedef struct task_tree_t {
    bool                 initialised;
    queue_t             *queue;
    pthread_mutex_t      lock;
    tree_node_t         *root_node;
    char                 graph_output[TREE_BUFFSZ + 1];
    char                 graph_nodeattr[TREE_BUFFSZ + 1];
    int                  graph_output_format;
} task_tree_t;

/* A single static instance of a task tree exists within the module. Access is
   made thread-safe through a mutex.
 */
static task_tree_t Tree = {
    .initialised  = false,
    .queue        = NULL,
    .lock         = PTHREAD_MUTEX_INITIALIZER,
    .root_node    = NULL,//{.parent_id = {.ptr = NULL}, .children = NULL},
    .graph_output = {0},
    .graph_nodeattr = {0},
    .graph_output_format   = format_dot
};

/* Initialise the task tree. Should be called once at program start.

   Allocates a queue for the tree's nodes. Allocates space for any children
   of the root node. Aborts execution if initialisation fails for any reason.
   Returns true on success. Does nothing if the tree is aleady initialised.

   This function is NOT thread-safe as it it intended to be called before
   any threads are created.
 */
bool 
tree_init(otter_opt_t *opt)
{
    if (Tree.initialised)
    {
        LOG_WARN("already initialised");
        return Tree.initialised;
    }

    LOG_INFO("initialising");

    /* allocate queue, root node & array of children */
    Tree.queue = queue_create(&tree_destroy_node);
    Tree.root_node = malloc(sizeof(*Tree.root_node));
    Tree.root_node->parent_id.value = 0;
    Tree.root_node->children = array_create(OTT_DEFAULT_ROOT_CHILDREN);

    /* if the tree's queue or root node were not created, abort execution */
    if ((Tree.queue == NULL) || (Tree.root_node->children == NULL))
    {
        LOG_ERROR("failed to initialise, aborting");
        abort();
    }

    /* Don't add root node to the queue so it isn't written to file */
    // queue_push(Tree.queue, (queue_item_t){.ptr=Tree.root_node});

    /* detect options set in environment */
    if ((opt->graph_output == NULL) || STR_EQUAL(opt->graph_output, ""))
        opt->graph_output = "OTTER-TASK-TREE";
    if ((opt->graph_nodeattr == NULL) || STR_EQUAL(opt->graph_nodeattr, ""))
        opt->graph_nodeattr = "OTTER-TASK-TREE-NODE-ATTR.csv";

    if (opt->graph_format == NULL) opt->graph_format = "not set";

    char *ext = ".gv";
    if (STR_EQUAL(opt->graph_format, "edge")) {
        Tree.graph_output_format = format_edge;
        ext = ".csv";
    } else if (STR_EQUAL(opt->graph_format, "adj")) {
        Tree.graph_output_format = format_adjacency;
        ext = ".json";
    } else {
        Tree.graph_output_format = format_dot;
        ext = ".gv";
    }

    char *pos = NULL;

    /* Copy graph output filename, extension and hostname */
    pos = &Tree.graph_output[0]; 
    strncpy(Tree.graph_output, opt->graph_output, TREE_BUFFSZ); 
    pos = &Tree.graph_output[0] + strlen(Tree.graph_output); 
    strncpy(pos, ext, TREE_BUFFSZ - strlen(Tree.graph_output)); 
    pos = &Tree.graph_output[0] + strlen(Tree.graph_output); 
    if (opt->append_hostname)
    { 
        strcpy(pos, "."); 
        strncpy(pos+1, opt->hostname,
            TREE_BUFFSZ - strlen(Tree.graph_output)); 
    }

    /* Copy graph output filename and hostname */
    pos = &Tree.graph_nodeattr[0]; 
    strncpy(Tree.graph_nodeattr, opt->graph_nodeattr, TREE_BUFFSZ);
    pos = &Tree.graph_nodeattr[0] + strlen(Tree.graph_nodeattr); 
    if (opt->append_hostname)
    { 
        strcpy(pos, "."); 
        strncpy(pos+1, opt->hostname,
            TREE_BUFFSZ - strlen(Tree.graph_nodeattr)); 
    }

    return Tree.initialised = true;
}

/* Create a tree node for a task and atomically add it to the tree's node queue.
   The array associated with the node has space for "children" IDs initially.
   Return the address of the tree node for the task to maintain.
   
   Aborts program execution if the tree is not initialised.

   Returns NULL if a tree node could not be created.

   Print an error message to the log if queueing the node fails.

   Addresses returned by this function should not be manually free'd, but will
   be free'd when the tree is written or destroyed.

 */
tree_node_t *
tree_add_node(tree_node_id_t parent_id, size_t n_children)
{
    /* lock the tree to ensure atomic access */
    pthread_mutex_lock(&Tree.lock);

    tree_node_t *node = NULL;

    /* unpack task type & parallel region bits from parent id */
    tree_node_id_t task_type, parallel_region_id;
    UNPACK_TASK_ID_BITS(task_type, parallel_region_id, parent_id.value);

    LOG_INFO("%-20s: 0x%016lx -> %lu, %lu, %lu", "PARENT ID UNPACKING",
        parent_id.value, task_type.value, parallel_region_id.value,
        parent_id.value & 0xFFFFFFFF);

    /* if tree not initialised when trying to add a node, abort execution */
    if (false == Tree.initialised)
    {
        LOG_ERROR("task tree not initialised before adding node, aborting");
        abort();
    }

    /* if node creation failed, exit safely (unlock mutex and return NULL) with
       message
    */
    if (NULL == (node = malloc(sizeof(*node))))
    {
        LOG_ERROR("failed to create node for parent %p", parent_id.ptr);
        goto unlock_and_exit;
    }

    node->parent_id = parent_id;

    /* if creation of node's array failed, exit safely with message */
    if (NULL == (node->children = array_create(n_children)))
    {
        LOG_ERROR("task tree failed to create array for parent %lu",
            parent_id.value);
        free(node);
        node = NULL;
        goto unlock_and_exit;
    }

    /* queue the new node */
    LOG_DEBUG("%lu queueing %p->%p", parent_id.value, node, node->children);
    bool node_was_queued = queue_push(Tree.queue, (queue_item_t){.ptr = node});

    #if DEBUG_LEVEL >= 4
    queue_print(Tree.queue);
    #endif

    /* if queueing failed, print a message */
    LOG_ERROR_IF(false == node_was_queued,
        "task tree failed to queue node %p for parent %lu",
        node, parent_id.value);

unlock_and_exit:

    /* in all cases, unlock the mutex and return final address of node (NULL 
       on error)
    */
    pthread_mutex_unlock(&Tree.lock);
    return node;
}

/* Add the ID of a child task to the node belonging to its parent to record
   that the parent spawned the child.

   If parent_node is NULL, atomically add the child to the tree's root node to
   indicate that the spawned task has no parent.

 */
bool 
tree_add_child_to_node(tree_node_t *parent_node, tree_node_id_t child_id)
{
    array_t *children = NULL;
    tree_node_t *parent = NULL;

    /* unpack task type & parallel region bits from child id */
    tree_node_id_t task_type, parallel_region_id;
    UNPACK_TASK_ID_BITS(task_type, parallel_region_id, child_id.value);

    LOG_INFO("%-20s: 0x%016lx -> %lu, %lu, %lu", "CHILD ID UNPACKING",
        child_id.value, task_type.value, parallel_region_id.value,
        child_id.value & 0xFFFFFFFF);

    /* child_id.value &= 0xFFFFFFFF; */

    /* Add a child to parent_node, or to the root node if it has no parent */
    if (parent_node == NULL)
    {
        pthread_mutex_lock(&Tree.lock);
        parent = Tree.root_node;
        children = Tree.root_node->children;
    } else {
        parent = parent_node;
        children = parent_node->children;
    }

    LOG_DEBUG("%lu -> %lu", parent->parent_id.value, child_id.value);

    /* Append the ID of the child. Write error on failure. */
    if (false == array_push_back(children, (array_element_t){.value = child_id.value}))
    {
        LOG_ERROR("task tree failed to add child %p to parent node %p",
            child_id.ptr, parent);

        /* Only need to unlock if we were adding to the root node */
        if (parent == Tree.root_node) pthread_mutex_unlock(&Tree.lock);
        return false;
    }

    #if DEBUG_LEVEL >= 4
    array_print(children);
    #endif
    
    /* Only need to unlock if we were adding to the root node */
    if (parent == Tree.root_node) pthread_mutex_unlock(&Tree.lock);
    return true;
}

/* Write the contents of the tree

   Note that this function has the side-effect of destroying the tree as nodes
   must be popped from the queue.

   Not thread-safe.

 */
bool 
tree_write(void)
{
    FILE *taskgraph = fopen(Tree.graph_output, "w");
    FILE *nodeattr  = fopen(Tree.graph_nodeattr, "w");
    LOG_INFO("output: \"%s\"", Tree.graph_output);
    LOG_INFO("output: \"%s\"", Tree.graph_nodeattr);

    if (taskgraph == NULL)
    {
        LOG_ERROR("failed to create file \"%s\": %s",
            Tree.graph_output, strerror(errno));
        errno = 0;
        return false;
    }

    if (nodeattr == NULL)
    {
        LOG_ERROR("failed to create file \"%s\": %s",
            Tree.graph_nodeattr, strerror(errno));
        errno = 0;
        return false;
    }
    
    int i=0;                          // loop counter
    size_t n_children = 0;            // # children of a given node in the queue
    array_element_t *child_tasks = NULL;  // children of a node
    tree_node_t *node = NULL;         // present node popped from the queue

    /* Write file headers */

    /* Attributes */
    fprintf(nodeattr, "node_id, task_type, parallel_region\n");

    /* Directed graph */
    switch (Tree.graph_output_format)
    {
        case format_dot:
            fprintf(taskgraph,
                "digraph \"\" {\n"
                "    graph   [fontname = \"helvetica\"];\n"
                "    node    [fontname = \"helvetica\" shape=record];\n"
                "    label = \"\";\n"
                "\n"
            );
            break;
        case format_edge:
            fprintf(taskgraph, "%s,%s\n", "parent_task_id", "child_task_id");
            break;
        case format_adjacency:
            fprintf(taskgraph, "{");
            break;
        default:
            LOG_ERROR("invalid output format identifier: %d",
                Tree.graph_output_format);
            fclose(taskgraph);
            fclose(nodeattr);
            return false;
    }

    /* Children of each node in the queue */
    while(queue_pop(Tree.queue, (queue_item_t*) &node))
    {
        LOG_ERROR_IF(node == NULL, "got null pointer from queue_pop");

        child_tasks = array_peek_data(node->children, &n_children);

        LOG_ERROR_IF(child_tasks == NULL,
            "got null pointer from array_peek_data (parent=%lu)",
            node->parent_id.value);

        LOG_DEBUG("parent=%lu (n=%lu)", node->parent_id.value, n_children);

        switch (Tree.graph_output_format)
        {
            case format_dot:
                for (i=0; i<n_children; i++)
                {
                    /* Directed edge */
                    fprintf(taskgraph, "%lu -> %lu\n", 
                        UNPACK_TASK_ID(node->parent_id.value),
                        UNPACK_TASK_ID(child_tasks[i].value));
                    /* Child formatting */
                    tree_write_dot_child_fmt(taskgraph,
                        (tree_node_id_t)child_tasks[i].value);
                    /* Attributes */
                    tree_write_child_node_attributes(nodeattr,
                        (tree_node_id_t)child_tasks[i].value);
                }
                break;
            case format_edge:
                for (i=0; i<n_children; i++)
                {
                    /* Directed edge */
                    fprintf(taskgraph, "%lu,%lu\n",
                        UNPACK_TASK_ID(node->parent_id.value),
                        UNPACK_TASK_ID(child_tasks[i].value));
                    /* Attributes */
                    tree_write_child_node_attributes(nodeattr,
                        (tree_node_id_t)child_tasks[i].value);
                }
                break;
            case format_adjacency:
                fprintf(taskgraph, "\n     \"%lu\" : [",
                    UNPACK_TASK_ID(node->parent_id.value));
                for (i=0; i<n_children; i++)
                {
                    /* Directed edge */
                    fprintf(taskgraph, "%s\"%lu\"", i==0 ?"":",",
                        UNPACK_TASK_ID(child_tasks[i].value));
                    /* Attributes */
                    tree_write_child_node_attributes(nodeattr,
                        (tree_node_id_t)child_tasks[i].value);
                }
                fprintf(taskgraph, "]");
                break;
            default:
                break;
        }
        tree_destroy_node(node);
    }

    /* Write file footer */
    switch (Tree.graph_output_format)
    {
        case format_dot:
        case format_adjacency:
            fprintf(taskgraph, "\n}\n");
            break;
        default:
            break;
    }
    
    fprintf(nodeattr, "\n");

    if (fclose(taskgraph) == 0)
        fprintf(stderr, "task tree written to \"%s\"\n", Tree.graph_output);
    else
        fprintf(stderr, "there was an error writing task tree to \"%s\"",
            Tree.graph_output);

    if (fclose(nodeattr) == 0)
        fprintf(stderr, "node attributes written to \"%s\"\n",
            Tree.graph_nodeattr);
    else
        fprintf(stderr, "there was an error writing node attributes to \"%s\"",
            Tree.graph_nodeattr);

    return true;
}

static void 
tree_write_dot_child_fmt(FILE *taskgraph, tree_node_id_t child_id)
{
    /* will unpack these values from the bits of each child ID */
    tree_node_id_t unpacked_id, task_type, parallel_region_id;

    /* a child task's node shape is determined by its OMP task type */
    char *child_node_shape = "box",
         *child_node_style = "solid",
         *child_node_colour = "black";

    unpacked_id.value = UNPACK_TASK_ID(child_id.value);

    UNPACK_TASK_ID_BITS(task_type, parallel_region_id, child_id.value);

    TASK_TYPE_TO_NODE_STYLE(task_type.value,
        child_node_style, child_node_shape, child_node_colour);

    LOG_INFO("%-20s: 0x%016lx -> %lu, %lu, %lu", "CHILD ID UNPACKING", 
        child_id.value, task_type.value,
        parallel_region_id.value, unpacked_id.value);
    
    fprintf(taskgraph,
        "  %lu [style= %s shape=%s color=%s]\n",
        unpacked_id.value, child_node_style, child_node_shape,
        child_node_colour);

    return;
}

static void
tree_write_child_node_attributes(FILE *nodeattr, tree_node_id_t child_id)
{
    /* Used for unpacking bits from children */
    tree_node_id_t task_id, task_type, parallel_region_id;

    task_id.value = UNPACK_TASK_ID(child_id.value);
    UNPACK_TASK_ID_BITS(task_type, parallel_region_id, child_id.value);

    fprintf(nodeattr, "%lu,%lu,%lu\n",
        task_id.value,
        task_type.value,
        parallel_region_id.value);

    return;
}

/* Destroy the task tree on program exit.

   De-allocates the tree's node queue and the array of children of the root
   node. Destroys the mutex protecting access to the tree and sets 
   Tree.initialised to false.

   Not thread-safe.

 */
void 
tree_destroy(void)
{
    if (Tree.initialised == false)
    {
        LOG_WARN("task tree not initialised");
        return;
    }

    LOG_INFO("destroying");

    /* destroy tree's node queue and the array of the root node */
    queue_destroy(Tree.queue, true);
    Tree.queue = NULL;
    pthread_mutex_destroy(&Tree.lock);

    Tree.initialised = false;

    return;
}

/* A callback for destroying an item in the tree's queue. Each item in the queue
   is a tree node, which contains the ID of the owning task and a dynamic array
   of the task's children.
 */
void
tree_destroy_node(void *ptr)
{
    tree_node_t *node = (tree_node_t*) ptr;
    LOG_DEBUG("%lu destroying %p->%p", node->parent_id.value, node, node->children);
    array_destroy(node->children);
    free(node);
    return;
}
