#if !defined(GRAPH_H)
#define GRAPH_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>
#include <otter-dtypes/queue.h>

/* define arbitrary node types */
typedef enum {_empty_enum_type} graph_node_type_t;

/* arbitrary data associated with a node */
typedef union {
    uint64_t    value;
    void       *ptr;
} graph_node_data_t;

/* opaque types */
typedef struct graph_t graph_t;
typedef struct graph_node_t graph_node_t;
typedef struct graph_edge_t graph_edge_t;

/* callback for destroying the data referenced by the nodes of a queue */
typedef void (*graph_free_node_data_t)(void*, graph_node_type_t);

/* create an empty graph */
graph_t *graph_create(void);

/* add a node to the graph with optional data */
graph_node_t *graph_add_node(
    graph_t *g, graph_node_type_t type, graph_node_data_t data);

/* declare an edge between two nodes (no attempt is made to check that the
   nodes already exist in the graph)
*/
graph_edge_t *graph_add_edge(graph_t *g, graph_node_t *n1, graph_node_t *n2);

/* move the nodes and edges of h into g */
bool graph_union(graph_t *g, graph_t *h);

/* destroy the graph
   if free!=NULL, call free(node->data, node->type) for each node
*/
void graph_destroy(graph_t *g, graph_free_node_data_t free_node_data);

/* scan the nodes of a graph without changing the graph */
void graph_scan_nodes(
    graph_t *g, uint64_t *id, graph_node_type_t *type, graph_node_data_t *data, void **next);

/* scan the edges of a graph without changing the graph */
void graph_scan_edges(
    graph_t *g,
    uint64_t           *src_id,
    graph_node_type_t  *src_type, 
    graph_node_data_t  *src_data, 
    uint64_t           *dest_id,
    graph_node_type_t  *dest_type, 
    graph_node_data_t  *dest_data,   
    void **next);

void graph_get_num_nodes_edges(graph_t *g, size_t *nodes, size_t *edges);

#if DEBUG_LEVEL >= 4
void graph_print(graph_t *g);
#endif

#endif // GRAPH_H
