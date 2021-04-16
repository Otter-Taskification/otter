#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <modules/task-tree.h>
#include <dtypes/queue.h>
#include <dtypes/dynamic-array.h>
#include <macros/debug.h>

/* Task tree - maintains queue of tree nodes */
typedef struct task_tree_t {
    bool                initialised;
    queue_t            *node_queue;
    pthread_mutex_t     lock;
} task_tree_t;

/* Task tree node - dynamic array of a task's children */
struct tt_node_t {
    tt_node_id_t      parent_id;
    dynamic_array_t  *children;
};

/* Global task tree - singleton */
static task_tree_t Tree = {
    .initialised = false,
    .node_queue = NULL
};

bool 
tt_init_tree(void)
{
    if (Tree.initialised)
    {
        LOG_WARN("tree already initialised");
        return Tree.initialised;
    }
    LOG_INFO("initialising task tree");
    if (NULL == (Tree.node_queue = qu_create()))
    {
        LOG_ERROR("failed to initialise task tree, aborting");
        abort();
    }
    int mutex_init_err = 0;
    if (0 != (mutex_init_err = pthread_mutex_init(&Tree.lock, NULL)))
    {
        LOG_ERROR("%s", strerror(mutex_init_err));
        return Tree.initialised;
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
        LOG_WARN("tree not initialised");
        return;
    }
    LOG_INFO("destroying task tree");
    qu_destroy(Tree.node_queue);
    Tree.node_queue = NULL;
    pthread_mutex_destroy(&Tree.lock);
    Tree.initialised = false;
    return;
}

tt_node_t *
tt_new_node(tt_node_id_t parent_id, size_t n_children)
{
    tt_node_t *node;
    if (NULL == (node = malloc(sizeof(*node))))
    {
        LOG_ERROR("failed to create node for parent %p", parent_id.ptr);
        return NULL;
    }
    node->parent_id = parent_id;
    if (NULL == (node->children = da_create(n_children)))
    {
        LOG_ERROR("failed to create dynamic array for parent %p",
            parent_id.ptr);
        free(node);
        return NULL;
    }

    pthread_mutex_lock(&Tree.lock);
    LOG_ERROR_IF(false == qu_enqueue(
            Tree.node_queue,(queue_data_t){.ptr = node}),
        "failed to add parent %p to task tree", parent_id.ptr);
    pthread_mutex_unlock(&Tree.lock);

    LOG_DEBUG("created node for parent %p with %lu elements",
        parent_id.ptr, n_children);
    return node;
}

bool 
tt_add_child_to_node(tt_node_t *parent_node, tt_node_id_t child_id)
{
    if (parent_node == NULL)
    {
        LOG_ERROR("failed to add child %p: null node pointer", child_id.ptr);
        return false;
    }
    if (false == da_push_back(parent_node->children,
        (array_element_t) {.ptr = child_id.ptr}))
    {
        LOG_ERROR("failed to add child %p to parent node %p",
            child_id.ptr, parent_node);
        return false;
    }
    LOG_DEBUG("added child %p to node %p (len=%lu)",
        child_id.ptr, parent_node, da_get_length(parent_node->children));
    return true;
}



