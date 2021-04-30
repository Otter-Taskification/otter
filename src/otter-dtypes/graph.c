#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>
#include <otter-dtypes/graph.h>

static uint64_t graph_get_unique_node_id(void);

struct graph_t {
    queue_t     *nodes;
    queue_t     *edges;
};

struct graph_node_t {
    uint64_t           id;
    graph_node_type_t  type;
    graph_node_data_t  data;
};

struct graph_edge_t {
    graph_node_t *src;
    graph_node_t *dest;
};

/* create an empty graph */
graph_t *
graph_create(void)
{
    graph_t *g = (graph_t*) malloc(szieof(*g));
    g->nodes = queue_create(NULL);
    g->edges = queue_create(NULL);
    return g;
}

/* add a node to the graph with optional data */
graph_node_t *
graph_add_node(
    graph_t             *g, 
    graph_node_type_t    type, 
    graph_node_data_t    data)
{
    if (g == NULL) return NULL;
    graph_node_t *n = (graph_node_t*) malloc(sizeof(*n));
    n->id = graph_get_unique_node_id();
    n->type = type;
    n->data = data;
    queue_push(g->nodes, (queue_item_t){.ptr = n});
    return n;
}

/* declare an edge between two nodes (no attempt is made to check that the
   nodes belong to this graph)
*/
graph_edge_t *
graph_add_edge(
    graph_t       *g, 
    graph_node_t  *n1, 
    graph_node_t  *n2)
{
    if ((g == NULL) || (n1 == NULL) || (n2 == NULL)) return NULL;
    graph_edge_t *e = (graph_edge_t*) malloc(sizeof(*e));
    e->src = n1, e->dest = n2;
    queue_push(g->edges, (queue_item_t){.ptr = e});
    return e;
}

/* move the nodes and edges of h into g */
bool
graph_union(
    graph_t  *g,
    graph_t  *h)
{
    if ((g == NULL) || (h == NULL)) return false;
    return queue_append(g->nodes, h->nodes) && queue_append(g->edges, h->edges);
}

/* destroy the graph and any nodes & edges it contains
   if free_node_data!=NULL, call free_node_data(node->data, node->type) 
    for each node
*/
void
graph_destroy(
    graph_t                *g, 
    graph_free_node_data_t  free_node_data)
{
    /* destroy queue of edges and the edges themselves */
    queue_destroy(g->edges, true);

    /* destroy node data if free != NULL */
    if (free_node_data != NULL)
    {
        graph_node_t *node = NULL;
        while(queue_pop(g->nodes, (queue_item_t*) &node))
        {
            free_node_data(node->data.ptr, node->type);
            free(node);
        }
    }

    queue_destroy(g->nodes, true);
    free(g);
    return;
}

static uint64_t
graph_get_unique_node_id(void)
{
    static uint64_t id = 0;
    return __sync_fetch_and_add(&id, 1L);
}