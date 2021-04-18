#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <modules/task-tree.h>
#include <modules/task-tree-graphviz.h>
#include <dtypes/queue.h>
#include <dtypes/dynamic-array.h>
#include <macros/debug.h>

/* A node is a container that belongs to a particular task. It maintains an
   array of the IDs of the child tasks spawned by the task
*/
struct tree_node_t {
    tree_node_id_t      parent_id;
    array_t            *children;
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
} task_tree_t;

/* A single static instance of a task tree exists within the module. Access is
   made thread-safe through a mutex.
 */
static task_tree_t Tree = {
    .initialised  = false,
    .queue        = NULL,
    .lock         = PTHREAD_MUTEX_INITIALIZER,
    .root_node    = {.parent_id = {.ptr = NULL}, .children = NULL}
};

/* A callback for destroying an item in the tree's queue. Each item in the queue
   is a tree node, which contains the ID of the owning task and a dynamic array
   of the task's children.
 */
void
destroy_tree_node(void *ptr)
{
    tree_node_t *node = (tree_node_t*) ptr;
    LOG_DEBUG("task tree callback: destroying tree node %p", node);
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
        LOG_WARN("task tree already initialised");
        return Tree.initialised;
    }

    LOG_INFO("task tree initialising");

    /* allocate queue and root node's array of children */
    Tree.queue = queue_create(&destroy_tree_node);
    Tree.root_node.children = array_create(DEFAULT_ROOT_CHILDREN);

    /* if the tree's queue or root node were not created, abort execution */
    if ((Tree.queue == NULL) || (Tree.root_node.children == NULL))
    {
        LOG_ERROR("task tree failed to initialise, aborting");
        abort();
    }

    return Tree.initialised = true;
}

/* Write the contents of the tree

   Note that this function has the side-effect of destroying the tree as nodes
   must be popped from the queue.

   Currently only outputs to stderr.

   Not thread-safe.

 */
bool 
tree_write(const char *fname)
{
    Agraph_t *graph = gv_new_graph(NULL);

    /* First write any children the root node has (it may have 0)... */
    size_t n_children = 0;

    /* Get the array of child IDs */
    array_element_t *child_ids = array_peek_data(
        Tree.root_node.children, &n_children);

    LOG_ERROR_IF(child_ids == NULL,
        "task tree got null pointer from root node");
    
    /* If there are any children, write their IDs */
    if (n_children > 0)
    {
        gv_add_children_to_graph(graph, Tree.root_node.parent_id,
            child_ids, n_children);
    }

    /* ... then write the children of each node in the queue */
    tree_node_t *node = NULL;

    /* queue_pop writes the popped item into &node */
    while(queue_pop(Tree.queue, (queue_item_t*) &node))
    {
        /* Get the array of child IDs */
        child_ids = array_peek_data(node->children, &n_children);

        LOG_ERROR_IF(child_ids == NULL,
            "task tree got null pointer from node id %p", node->parent_id.ptr);

        /* Add the parent task and its children to the graph */
        gv_add_children_to_graph(graph, node->parent_id,
            child_ids, n_children);

        /* destroy the node & the array it contains */
        destroy_tree_node(node);
    }

    gv_write_graph(graph);
    gv_finalise(graph);

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

    LOG_INFO("destroying task tree");

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
        LOG_ERROR("task tree failed to create dynamic array for parent %p",
            parent_id.ptr);
        free(node);
        node = NULL;
        goto unlock_and_exit;
    }

    LOG_DEBUG("task tree created node for parent %p with %lu elements",
        parent_id.ptr, n_children);

    /* queue the new node */
    bool node_was_queued = queue_push(Tree.queue, (queue_item_t){.ptr = node});

    #if DEBUG_LEVEL >= 4
    queue_print(Tree.queue);
    #endif

    /* if queueing failed, print a message */
    LOG_ERROR_IF(false == node_was_queued,
        "task tree failed to add parent %p", parent_id.ptr);


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
        LOG_DEBUG("task tree appending child %p to root node", child_id.ptr);
        pthread_mutex_lock(&Tree.lock);
        parent = &Tree.root_node;
        children = Tree.root_node.children;
    } else {
        parent = parent_node;
        children = parent_node->children;
    }

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

    LOG_DEBUG("task tree added child %p to node %p (len=%lu)",
        child_id.ptr, parent, array_length(parent->children));
    
    /* Only need to unlock if we were adding to the root node */
    if (parent == &Tree.root_node) pthread_mutex_unlock(&Tree.lock);
    return true;
}



