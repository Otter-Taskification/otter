#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <otter-task-tree/task-tree.h>

#include <otter-dtypes/queue.h>
#include <otter-dtypes/dynamic-array.h>

#include <macros/debug.h>
#include <macros/general.h>

static bool tree_write_dot(FILE *taskgraph);
static bool tree_write_edge_list(FILE *taskgraph);
static bool tree_write_adjacency_list(FILE *taskgraph);

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
    tree_node_t          root_node;
    char                 graph_output[TREE_OUTNAME_BUFSZ + 1];
    int                  graph_output_format;
} task_tree_t;

/* A single static instance of a task tree exists within the module. Access is
   made thread-safe through a mutex.
 */
static task_tree_t Tree = {
    .initialised  = false,
    .queue        = NULL,
    .lock         = PTHREAD_MUTEX_INITIALIZER,
    .root_node    = {.parent_id = {.ptr = NULL}, .children = NULL},
    .graph_output = {0},
    .graph_output_format   = format_dot
};

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

/* Initialise the task tree. Should be called once at program start.

   Allocates a queue for the tree's nodes. Allocates space for any children
   of the root node. Aborts execution if initialisation fails for any reason.
   Returns true on success. Does nothing if the tree is aleady initialised.

   This function is NOT thread-safe as it it intended to be called before
   any threads are created.
 */
bool 
tree_init(void)
{
    if (Tree.initialised)
    {
        LOG_WARN("already initialised");
        return Tree.initialised;
    }

    LOG_INFO("initialising");

    /* allocate queue and root node's array of children */
    Tree.queue = queue_create(&tree_destroy_node);
    Tree.root_node.children = array_create(OTT_DEFAULT_ROOT_CHILDREN);

    /* if the tree's queue or root node were not created, abort execution */
    if ((Tree.queue == NULL) || (Tree.root_node.children == NULL))
    {
        LOG_ERROR("failed to initialise, aborting");
        abort();
    }

    /* detect environment variables for graph output file */
    char *graph_output = getenv("OTTER_TASK_TREE_OUTPUT");
    char *graph_format = getenv("OTTER_TASK_TREE_FORMAT");

    LOG_INFO("OTTER_TASK_TREE_OUTPUT=%s", graph_output);
    LOG_INFO("OTTER_TASK_TREE_FORMAT=%s", graph_format);

    if ((graph_output == NULL) || STR_EQUAL(graph_output, ""))
        graph_output = "OTTer.dot";

    if (graph_format == NULL) graph_format = "not set";

    char *ext = ".dot";
    if (STR_EQUAL(graph_format, "edge")) {
        Tree.graph_output_format = format_edge;
        ext = ".csv";
    } else if (STR_EQUAL(graph_format, "adj")) {
        Tree.graph_output_format = format_adjacency;
        ext = ".json";
    } else {
        Tree.graph_output_format = format_dot;
        ext = ".dot";
    }

    strncpy(Tree.graph_output, graph_output,
        min(strlen(graph_output), TREE_OUTNAME_BUFSZ));
    
    strncpy(
        Tree.graph_output + 
            min(strlen(Tree.graph_output), TREE_OUTNAME_BUFSZ - strlen(ext)),
        ext, strlen(ext)
    );
    
    LOG_INFO("OUTPUT=%s", Tree.graph_output);

    return Tree.initialised = true;
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
    LOG_INFO("output: \"%s\"", Tree.graph_output);

    if (taskgraph == NULL)
    {
        LOG_ERROR("failed to create file: \"%s\"", strerror(errno));
        errno = 0;
        return false;
    }

    bool write_result = false;
    switch (Tree.graph_output_format)
    {
        case format_dot:
            write_result = tree_write_dot(taskgraph);
            break;
        case format_edge:
            write_result = tree_write_edge_list(taskgraph);
            break;
        case format_adjacency:
            write_result = tree_write_adjacency_list(taskgraph);
            break;
        default:
            LOG_ERROR("invalid output format identifier: %d",
                Tree.graph_output_format);
    }

    if (fclose(taskgraph) == 0)
    {
        fprintf(stderr, "task tree written to \"%s\"\n", Tree.graph_output);
    }
    return write_result;
}

static bool tree_write_dot(FILE *taskgraph)
{
    int i=0;

    /* Write digraph header */
    fprintf(taskgraph, 
        "digraph \"%s\" {                                                    \n"
        "    graph   [fontname = \"helvetica\"];                             \n"
        "    node    [fontname = \"helvetica\" shape=record];                \n"
        "    label = \"%s\";                                                 \n"
        "\n",
        Tree.graph_output,
        Tree.graph_output
    );

    /* First write any children the root node has (it may have 0)... */
    size_t n_children = 0;

    /* Get the array of child IDs */
    array_element_t *child_ids = array_peek_data(
        Tree.root_node.children, &n_children);

    LOG_ERROR_IF(child_ids == NULL, "got null pointer from array_peek_data");
    
    /* If there are any children, write their IDs */
    LOG_DEBUG("parent=(root) (n=%lu)", n_children);
    if (n_children > 0)
    {
        fprintf(taskgraph, "%lu -> {", Tree.root_node.parent_id.value);
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%s%lu", i==0 ? "" : ",", child_ids[i].value); 
        }
        fprintf(taskgraph, "}\n");
    }

    /* ... then write the children of each node in the queue */
    tree_node_t *node = NULL;

    /* queue_pop writes the popped item into &node */
    while(queue_pop(Tree.queue, (queue_item_t*) &node))
    {
        LOG_ERROR_IF(node == NULL, "got null pointer from queue_pop");

        /* Get the array of child IDs */
        child_ids = array_peek_data(node->children, &n_children);
        LOG_ERROR_IF(child_ids == NULL,
            "got null pointer from array_peek_data (parent=%lu)",
            node->parent_id.value);

        LOG_DEBUG("parent=%lu (n=%lu)", node->parent_id.value, n_children);

        /* Add the parent task and its children to the graph */
        fprintf(taskgraph, "%lu -> {", node->parent_id.value);
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%s%lu", i==0 ? "" : ",", child_ids[i].value); 
        }
        fprintf(taskgraph, "}\n");

        /* destroy the node & the array it contains */
        tree_destroy_node(node);
    }

    /* Close the digraph */
    fprintf(taskgraph, "\n}\n");
    return true;
}

static bool tree_write_edge_list(FILE *taskgraph)
{
    int i=0; 

    fprintf(taskgraph, "%s,%s\n", "parent_task_id", "child_task_id");

    /* First write any children the root node has (it may have 0)... */
    size_t n_children = 0;

    /* Get the array of child IDs */
    array_element_t *child_ids = array_peek_data(
        Tree.root_node.children, &n_children);

    LOG_ERROR_IF(child_ids == NULL, "got null pointer from array_peek_data");
    
    /* If there are any children, write their IDs */
    LOG_DEBUG("parent=(root) (n=%lu)", n_children);
    if (n_children > 0)
    {
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%lu,%lu\n",
                Tree.root_node.parent_id.value, child_ids[i].value); 
        }
    }

    /* ... then write the children of each node in the queue */
    tree_node_t *node = NULL;

    /* queue_pop writes the popped item into &node */
    while(queue_pop(Tree.queue, (queue_item_t*) &node))
    {
        LOG_ERROR_IF(node == NULL, "got null pointer from queue_pop");

        /* Get the array of child IDs */
        child_ids = array_peek_data(node->children, &n_children);
        LOG_ERROR_IF(child_ids == NULL,
            "got null pointer from array_peek_data (parent=%lu)",
            node->parent_id.value);

        LOG_DEBUG("parent=%lu (n=%lu)", node->parent_id.value, n_children);

        /* Add the parent task and its children to the graph */
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%lu,%lu\n",
                node->parent_id.value, child_ids[i].value); 
        }

        /* destroy the node & the array it contains */
        tree_destroy_node(node);
    }

    return true;
}

static bool tree_write_adjacency_list(FILE *taskgraph)
{
    int i=0;

    /* Write header */
    fprintf(taskgraph, "{\n");

    /* First write any children the root node has (it may have 0)... */
    size_t n_children = 0;

    /* Get the array of child IDs */
    array_element_t *child_ids = array_peek_data(
        Tree.root_node.children, &n_children);

    LOG_ERROR_IF(child_ids == NULL, "got null pointer from array_peek_data");
    
    /* If there are any children, write their IDs */
    LOG_DEBUG("parent=(root) (n=%lu)", n_children);
    if (n_children > 0)
    {
        fprintf(taskgraph, "    \"%lu\" : [", Tree.root_node.parent_id.value);
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%s\"%lu\"", i==0 ? "" : ",", child_ids[i].value); 
        }
        fprintf(taskgraph, "]");
    }

    /* ... then write the children of each node in the queue */
    tree_node_t *node = NULL;

    /* queue_pop writes the popped item into &node */
    while(queue_pop(Tree.queue, (queue_item_t*) &node))
    {
        LOG_ERROR_IF(node == NULL, "got null pointer from queue_pop");

        /* Get the array of child IDs */
        child_ids = array_peek_data(node->children, &n_children);
        LOG_ERROR_IF(child_ids == NULL,
            "got null pointer from array_peek_data (parent=%lu)",
            node->parent_id.value);

        LOG_DEBUG("parent=%lu (n=%lu)", node->parent_id.value, n_children);

        /* Add the parent task and its children to the graph */
        fprintf(taskgraph, ",\n    \"%lu\" : [", node->parent_id.value);
        for (i=0; i<n_children; i++)
        {
            fprintf(taskgraph, "%s\"%lu\"", i==0 ? "" : ",", child_ids[i].value); 
        }
        fprintf(taskgraph, "]");

        /* destroy the node & the array it contains */
        tree_destroy_node(node);
    }

    /* Close the file */
    fprintf(taskgraph, "\n}\n");
    return true;
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
    array_destroy(Tree.root_node.children);
    Tree.root_node.children = NULL;
    
    pthread_mutex_destroy(&Tree.lock);

    Tree.initialised = false;

    return;
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

    /* Add a child to parent_node, or to the root node if it has no parent */
    if (parent_node == NULL)
    {
        pthread_mutex_lock(&Tree.lock);
        parent = &Tree.root_node;
        children = Tree.root_node.children;
    } else {
        parent = parent_node;
        children = parent_node->children;
    }

    LOG_DEBUG("%lu -> %lu", parent->parent_id.value, child_id.value);

    /* Append the ID of the child. Write error on failure. */
    if (false == array_push_back(children, (array_element_t){.ptr = child_id.ptr}))
    {
        LOG_ERROR("task tree failed to add child %p to parent node %p",
            child_id.ptr, parent);

        /* Only need to unlock if we were adding to the root node */
        if (parent == &Tree.root_node) pthread_mutex_unlock(&Tree.lock);
        return false;
    }

    #if DEBUG_LEVEL >= 4
    array_print(children);
    #endif
    
    /* Only need to unlock if we were adding to the root node */
    if (parent == &Tree.root_node) pthread_mutex_unlock(&Tree.lock);
    return true;
}
