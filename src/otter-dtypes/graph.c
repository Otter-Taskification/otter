#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>
#include <otter-dtypes/graph.h>

struct graph_t {
    queue_t         *nodes;
    queue_t         *edges;
    graph_node_t     root;
};

struct graph_node_t {
    uint64_t     id;
    node_type_t  type;
    node_data_t  data;
};

struct graph_edge_t {
    graph_node_t    *src;
    graph_node_t    *dest;
};
