#if !defined(TASK_TREE_GVIZ_H)
#define TASK_TREE_GVIZ_H

#include <stdbool.h>

#include <gvc.h>
#include <modules/task-tree.h>
#include <otter-dt/dynamic-array.h>
#include <otter-dt/queue.h>
#include <macros/debug.h>

/* Functionality */
Agraph_t   *gv_new_graph(const char *fname);
bool        gv_add_children_to_graph(Agraph_t *g, tree_node_id_t parent_id, array_element_t *child_ids, size_t length);
bool        gv_write_graph(Agraph_t *g);
bool        gv_finalise(Agraph_t *g);

#endif // TASK_TREE_GVIZ_H
