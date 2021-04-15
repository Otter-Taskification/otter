#include <stdlib.h>
#include <pthread.h>

#include <modules/task-tree.h>

/* Task tree - maintains queue of tree nodes */
typedef struct task_tree_t task_tree_t;

bool 
tt_init_tree(void)
{
    return false;
}

bool 
tt_write_tree(const char *fname)
{
    return false;
}

bool 
tt_destroy_tree(void)
{
    return false
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



