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
    Tree.node_queue = qu_create();
    if (Tree.node_queue == NULL)
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
tt_new_node(tt_node_id_t node_ref)
{
    return NULL;
}

bool 
tt_add_child_to_node(tt_node_t *node, tt_node_id_t child_ref)
{
    return false;
}



