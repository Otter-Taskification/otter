#if !defined(TASK_TREE_H)
#define TASK_TREE_H

#include <stdbool.h>
#include <stdint.h>

/* Default number of child references the root node starts with */
#define DEFAULT_ROOT_CHILDREN 1000

/* Task tree node - maintains list of its child nodes */
typedef struct tt_node_t tt_node_t;

/* Node identifier */
typedef union tt_node_id_t {
    void        *ptr;
    uint64_t     value;
} tt_node_id_t;

// task tree functions
bool tt_init_tree(void);
bool tt_write_tree(const char *fname);
void tt_destroy_tree(void);

// task tree node functions
tt_node_t *tt_new_node(tt_node_id_t parent_id, size_t n_children);
bool tt_add_child_to_node(tt_node_t *parent_node, tt_node_id_t child_id);

#endif // TASK_TREE_H
