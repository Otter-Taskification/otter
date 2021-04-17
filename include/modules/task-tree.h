#if !defined(TASK_TREE_H)
#define TASK_TREE_H

#include <stdbool.h>
#include <stdint.h>

/* Default number of child references the root node starts with */
#define DEFAULT_ROOT_CHILDREN 1000

/* Task tree node - maintains list of its child nodes */
typedef struct tree_node_t tree_node_t;

/* Node identifier */
typedef union tree_node_id_t {
    void        *ptr;
    uint64_t     value;
} tree_node_id_t;

// task tree functions
bool         tree_init(void);
bool         tree_write(const char *fname);
void         tree_destroy(void);

// task tree node functions
tree_node_t *tree_add_node(tree_node_id_t parent_id, size_t n_children);
bool         tree_add_child_to_node(tree_node_t *parent_node, 
                tree_node_id_t child_id);

#endif // TASK_TREE_H
