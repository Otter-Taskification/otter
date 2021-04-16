#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <modules/task-tree.h>
#include <dtypes/queue.h>
#include <dtypes/dynamic-array.h>
#include <macros/debug.h>

/* Task tree node - dynamic array of a task's children */
struct tt_node_t {
    tt_node_id_t      parent_id;
    dynamic_array_t  *children;
};

/* Task tree - maintains queue of tree nodes */
typedef struct task_tree_t {
    bool                initialised;
    queue_t            *node_queue;
    pthread_mutex_t     lock;
    tt_node_t           root_node;
} task_tree_t;

/* Global task tree - singleton */
static task_tree_t Tree = {
    .initialised = false,
    .node_queue = NULL,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .root_node = {.parent_id = {.ptr = NULL}, .children = NULL}
};

void destroy_tree_node(void *ptr)
{
    tt_node_t *node = (tt_node_t*) ptr;
    LOG_DEBUG("task tree callback: destroying tree node %p", node);
    da_destroy(node->children);
    free(node);
    return;
}

bool 
tt_init_tree(void)
{
    if (Tree.initialised)
    {
        LOG_WARN("task tree already initialised");
        return Tree.initialised;
    }
    LOG_INFO("task tree initialising");
    Tree.node_queue = qu_create(&destroy_tree_node);
    Tree.root_node.children = da_create(DEFAULT_ROOT_CHILDREN);
    if ((Tree.node_queue == NULL) || (Tree.root_node.children == NULL))
    {
        LOG_ERROR("task tree failed to initialise, aborting");
        abort();
    }
    return Tree.initialised = true;
}

bool 
tt_write_tree(const char *fname)
{
    LOG_ERROR("TODO");
    return false;
}

void 
tt_destroy_tree(void)
{
    if (Tree.initialised == false)
    {
        LOG_WARN("task tree not initialised");
        return;
    }
    LOG_INFO("destroying task tree");
    qu_destroy(Tree.node_queue, true, true);
    da_destroy(Tree.root_node.children);
    Tree.node_queue = NULL;
    Tree.root_node.children = NULL;
    pthread_mutex_destroy(&Tree.lock);
    Tree.initialised = false;
    return;
}

tt_node_t *
tt_new_node(tt_node_id_t parent_id, size_t n_children)
{
    pthread_mutex_lock(&Tree.lock);
    tt_node_t *node = NULL;
    if (false == Tree.initialised)
    {
        LOG_ERROR("task tree not initialised before adding node, aborting");
        abort();
    }
    if (NULL == (node = malloc(sizeof(*node))))
    {
        LOG_ERROR("failed to create node for parent %p", parent_id.ptr);
        goto unlock_and_exit;
    }
    node->parent_id = parent_id;
    if (NULL == (node->children = da_create(n_children)))
    {
        LOG_ERROR("task tree failed to create dynamic array for parent %p",
            parent_id.ptr);
        free(node);
        node = NULL;
        goto unlock_and_exit;
    }
    LOG_DEBUG("task tree created node for parent %p with %lu elements",
        parent_id.ptr, n_children);
    bool node_was_enqueued = qu_enqueue(
            Tree.node_queue, (queue_data_t){.ptr = node});
    LOG_ERROR_IF(false == node_was_enqueued,
        "task tree failed to add parent %p", parent_id.ptr);

unlock_and_exit:
    pthread_mutex_unlock(&Tree.lock);
    return node;
}

bool 
tt_add_child_to_node(tt_node_t *parent_node, tt_node_id_t child_id)
{
    dynamic_array_t *children = NULL;
    if (parent_node == NULL)
    {
        LOG_DEBUG("task tree appending child %p to root node", child_id.ptr);
        pthread_mutex_lock(&Tree.lock);
        children = Tree.root_node.children;
    } else {
        children = parent_node->children;
    }
    if (false == da_push_back(children, (array_element_t){.ptr = child_id.ptr}))
    {
        LOG_ERROR("task tree failed to add child %p to parent node %p",
            child_id.ptr, parent_node);
        if (parent_node == NULL) pthread_mutex_unlock(&Tree.lock);
        return false;
    }
    LOG_DEBUG("task tree added child %p to node %p (len=%lu)",
        child_id.ptr, parent_node, da_get_length(parent_node->children));
    if (parent_node == NULL) pthread_mutex_unlock(&Tree.lock);
    return true;
}



