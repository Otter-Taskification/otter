#if !defined(TASK_TREE_H)
#define TASK_TREE_H

#include <stdbool.h>
#include <stdint.h>
#include <otter-core/ompt-common.h>
#include <macros/debug.h>

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

/* Default number of child references the root node starts with */
#if !defined(OTT_DEFAULT_ROOT_CHILDREN) \
    || (EXPAND(OTT_DEFAULT_ROOT_CHILDREN) == 1)
#undef OTT_DEFAULT_ROOT_CHILDREN
#define OTT_DEFAULT_ROOT_CHILDREN 1000
#endif

#define TREE_BUFFSZ                        512
#define TASK_TREE_DEFAULT_GRAPH_NAME       "OTTER-TASK-TREE"
#define TASK_TREE_DEFAULT_GRAPH_ATTR_NAME  "OTTER-TASK-TREE-NODE-ATTR.csv"

/* unpack child task bits to get task type & enclosing parallel region

   see PACK_TASK_BITS in ompt-callback-macros.h
 */
#define UNPACK_TASK_ID_BITS(type_var, par_id_var, task_id)                     \
    do {                                                                       \
        type_var   = (tree_node_id_t) (task_id>>60);                           \
        par_id_var = (tree_node_id_t) ((task_id>>48)&0xFF);                    \
    } while (0)

#define UNPACK_TASK_ID(task_id_value) (task_id_value & 0xFFFFFFFF)

#define TASK_TYPE_TO_NODE_STYLE(task_type, node_style, node_shape, node_colour)\
do{                                                                            \
    switch (task_type)                                                         \
    {                                                                          \
        case ompt_task_initial:                                                \
            node_style = "filled";                                             \
            node_shape = "diamond";                                            \
            node_colour = "lightgrey";                                         \
            break;                                                             \
        case ompt_task_implicit:                                               \
            node_style = "filled";                                             \
            node_shape = "circle";                                             \
            node_colour = "yellow";                                            \
            break;                                                             \
        case ompt_task_explicit:                                               \
            node_style = "solid";                                              \
            node_shape = "box";                                                \
            node_colour = "red";                                               \
            break;                                                             \
        case ompt_task_target:                                                 \
            node_style = "solid";                                              \
            node_shape = "hexagon";                                            \
            node_colour = "black";                                             \
            break;                                                             \
        default:                                                               \
            node_style = "solid";                                              \
            node_shape = "none";                                               \
            node_colour = "black";                                             \
    }                                                                          \
} while(0)

/* Task tree node - maintains list of its child nodes */
typedef struct tree_node_t tree_node_t;

/* Node identifier */
typedef union tree_node_id_t {
    void        *ptr;
    uint64_t     value;
} tree_node_id_t;

// task tree functions
bool         tree_init(otter_opt_t *opt);
bool         tree_write(void);
void         tree_destroy(void);

// task tree node functions
tree_node_t *tree_add_node(tree_node_id_t parent_id, size_t n_children);
bool         tree_add_child_to_node(tree_node_t *parent_node, 
                tree_node_id_t child_id);

#endif // TASK_TREE_H
