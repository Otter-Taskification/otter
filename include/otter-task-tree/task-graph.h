#if !defined(OTTER_TASK_GRAPH_H)
#define OTTER_TASK_GRAPH_H

#include <stdbool.h>
#include <stdint.h>
#include <otter-core/ompt-common.h>
#include <otter-dtypes/graph.h>
#include <macros/debug.h>

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

#define TASK_GRAPH_BUFFSZ                   512
#define TASK_GRAPH_DEFAULT_GRAPH_NAME       "OTTER_TASK_GRAPH"
#define TASK_GRAPH_DEFAULT_GRAPH_ATTR_NAME  "OTTER_TASK_GRAPH_NODE_ATTR.csv"
#define PID_STR_SZ 32

/* Alias graph types to distinguish graph elements created/owned by task graph
   from those used to build temporary sub-graphs which are eventually added to
   the task graph.
*/
typedef graph_node_t         task_graph_node_t;
typedef graph_node_data_t    task_graph_node_data_t;

/* Represent arbitrary node types the task graph can contain */
#define FLAG_NODE_TYPE_END(f) (f & 0x1000)
typedef enum {

    /* OMP task types */
    node_task_initial,
    node_task_implicit,
    node_task_explicit,
    node_task_target,

    /* Parallel region context */
    node_context_parallel_begin,

    /* Worksharing Contexts */
    node_context_sections_begin,
    node_context_single_begin,

    /* Worksharing-loop Contexts */
    node_context_loop_begin,
    node_context_taskloop_begin,

    /* Synchronisation Contexts */
    node_context_sync_taskgroup_begin,

    /* Matching endpoints - switch on a flag for these */
    node_context_parallel_end = FLAG_NODE_TYPE_END(node_context_parallel_begin),
    node_context_sections_end,
    node_context_single_end,
    node_context_loop_end,
    node_context_taskloop_end,
    node_context_sync_taskgroup_end,

    /* Standalone (i.e. never nested) synchronisation directives
       (not contexts)
       restart numbering from before flag
    */
    node_sync_barrier = node_context_sync_taskgroup_begin + 1,
    node_sync_barrier_implicit,
    node_sync_barrier_explicit,
    node_sync_barrier_implementation,
    node_sync_taskwait

    /* etc... */
} task_graph_node_type_t;

// task graph functions
bool task_graph_init(otter_opt_t *opt);
bool task_graph_write(void);
void task_graph_destroy(graph_free_node_data_t free_node_data);

/* add a node to the graph and return a reference to it */
task_graph_node_t *task_graph_add_node(
    task_graph_node_type_t node_type, task_graph_node_data_t node_data);

/* declare an edge from src to dest */
void task_graph_add_edge(task_graph_node_t *src, task_graph_node_t *dest);

/* move the nodes and edges from the subgraph into the task graph. does not
   create any edges between the two graphs. after this operation subgraph will
   be empty
*/
bool task_graph_attach_subgraph(graph_t *subgraph);

/* TODO:
    - write node attributes to json (differentiate fields by node type)
 */

#endif // OTTER_TASK_GRAPH_H
