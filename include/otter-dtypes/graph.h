#if !defined(GRAPH_H)
#define GRAPH_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>
#include <otter-dtypes/queue.h>

/* define arbitrary node types */
typedef int node_type_t;

/* arbitrary data associated with a node */
typedef union {
    uint64_t    value;
    void       *ptr;
} node_data_t;

/* opaque types */
typedef struct graph_t graph_t;
typedef struct graph_node_t graph_node_t;
typedef struct graph_edge_t graph_edge_t;

/* callback for destroying the data referenced by the nodes of a queue */
typedef void (*graph_free_node_data_t)(void*, node_type_t);

/* create an empty graph */
graph_t *graph_create(void);

/* add a node to the graph with optional data */
graph_node_t *graph_add_node(
    graph_t *g, uint64_t id, node_type_t type, node_data_t data);

/* declare an edge between two nodes (no attempt is made to check that the
   nodes already exist in the graph)
*/
graph_edge_t *graph_add_edge(graph_t *g, graph_node_t *s, graph_node_t *d);

/* include references to the nodes and edges from h in g (shallow copy) */
bool graph_union(graph_t *g, graph_t *h);

/* destroy the graph
    if deep==true, destroy nodes and edges also
    if free!=NULL, call free(node->data, node->type) for each node
        (requires deep==true)
*/
graph_t *graph_destroy(graph_t *g, bool deep, graph_free_node_data_t free);

#endif // GRAPH_H
