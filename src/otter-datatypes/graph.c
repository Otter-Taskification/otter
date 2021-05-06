#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <macros/debug.h>
#include <otter-datatypes/graph.h>

static uint64_t graph_get_unique_node_id(void);

struct graph_t {
    queue_t     *nodes;
    queue_t     *edges;
};

struct graph_node_t {
    uint64_t           id;
    graph_node_type_t  type;
    graph_node_data_t  data;
    bool               has_children;
};

struct graph_edge_t {
    graph_node_t *src;
    graph_node_t *dest;
};

/* create an empty graph */
graph_t *
graph_create(void)
{
    graph_t *g = (graph_t*) malloc(sizeof(*g));
    g->nodes = queue_create();
    g->edges = queue_create();
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
    n->has_children = false;
    queue_push(g->nodes, (data_item_t){.ptr = n});
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
    n1->has_children = true;
    queue_push(g->edges, (data_item_t){.ptr = e});
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
    queue_destroy(g->edges, true, NULL);

    /* destroy node data if free_node_data != NULL */
    graph_node_t *node = NULL;
    while(queue_pop(g->nodes, (data_item_t*) &node))
    {
        if (free_node_data != NULL)
            free_node_data(node->data.ptr, node->type);
        free(node);
    }

    queue_destroy(g->nodes, true, NULL);
    free(g);
    return;
}

/* scan the nodes of a graph without changing the graph */
void graph_scan_nodes(
    graph_t             *g,
    uint64_t            *id,
    graph_node_type_t   *type,
    graph_node_data_t   *data,
    void               **next)
{
    if ((g == NULL) || (next == NULL))
    {
        LOG_ERROR("null pointer");
        return;
    }
    graph_node_t *node = NULL;
    queue_scan(g->nodes, (data_item_t*) &node, next);
    if (id   ) *id   = node->id;
    if (type ) *type = node->type;
    if (data ) *data = node->data;
    return;
}

void 
graph_scan_edges(
    graph_t            *g,
    uint64_t           *src_id,
    graph_node_type_t  *src_type, 
    graph_node_data_t  *src_data, 
    uint64_t           *dest_id,
    graph_node_type_t  *dest_type, 
    graph_node_data_t  *dest_data,     
    void              **next)
{
    if ((g == NULL) || (next == NULL))
    {
        LOG_ERROR("null pointer");
        return;
    }
    graph_edge_t *edge = NULL;
    queue_scan(g->edges, (data_item_t*) &edge, next);
    if (src_id    ) *src_id    = edge->src->id;
    if (src_type  ) *src_type  = edge->src->type; 
    if (src_data  ) *src_data  = edge->src->data; 
    if (dest_id   ) *dest_id   = edge->dest->id; 
    if (dest_type ) *dest_type = edge->dest->type; 
    if (dest_data ) *dest_data = edge->dest->data;  
    return;
}

bool
graph_pop_node(
    graph_t            *g, 
    uint64_t           *id,
    graph_node_type_t  *type, 
    graph_node_data_t  *data)
{
    if (g == NULL)
    {
        LOG_ERROR("null pointer");
        return false;
    }
    graph_node_t *node = NULL;
    bool result;
    if ((result = queue_pop(g->nodes, (data_item_t*) &node))) {
        if (id   ) *id   = node->id;
        if (type ) *type = node->type;
        if (data ) *data = node->data;
    }
    free(node);
    return result;
}

bool
graph_pop_edge(
    graph_t            *g,
    uint64_t           *src_id,
    graph_node_type_t  *src_type, 
    graph_node_data_t  *src_data, 
    uint64_t           *dest_id,
    graph_node_type_t  *dest_type, 
    graph_node_data_t  *dest_data)
{
    if (g == NULL)
    {
        LOG_ERROR("null pointer");
        return false;
    }
    graph_edge_t *edge = NULL;
    bool result;
    if ((result = queue_pop(g->edges, (data_item_t*) &edge)))
    {
        if (src_id    ) *src_id    = edge->src->id;
        if (src_type  ) *src_type  = edge->src->type; 
        if (src_data  ) *src_data  = edge->src->data; 
        if (dest_id   ) *dest_id   = edge->dest->id; 
        if (dest_type ) *dest_type = edge->dest->type; 
        if (dest_data ) *dest_data = edge->dest->data;  
    }
    free(edge);
    return result;
}

void 
graph_get_num_nodes_edges(
    graph_t *g,
    size_t *nodes,
    size_t *edges)
{
    if ((g == NULL) || (nodes == NULL) || (edges == NULL))
    {
        LOG_ERROR("null pointer");
        return;
    }
    *nodes = queue_length(g->nodes);
    *edges = queue_length(g->edges);
    return;
}

bool
graph_node_has_children(graph_node_t *n)
{
    return n == NULL ? false : n->has_children;
}

#if DEBUG_LEVEL >= 4
void
graph_print(graph_t *g)
{
    fprintf(stderr, "GRAPH NODES %p->%p\n", g, g->nodes);
    queue_print(g->nodes);
    fprintf(stderr, "GRAPH EDGES %p->%p\n", g, g->edges);
    queue_print(g->edges);
}
#endif

static uint64_t
graph_get_unique_node_id(void)
{
    static uint64_t id = 0;
    return __sync_fetch_and_add(&id, 1L);
}