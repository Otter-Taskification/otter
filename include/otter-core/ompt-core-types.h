#if !defined(OMPT_CORE_TYPES_H)
#define OMPT_CORE_TYPES_H

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

#include <pthread.h>

#include <otter-core/ompt-common.h>
#include <otter-dtypes/stack.h>
#include <otter-task-tree/task-tree.h>
#include <otter-task-tree/task-graph.h>
#include <otter-trace/trace.h>

/* forward declarations */
typedef struct parallel_data_t parallel_data_t;
typedef struct thread_data_t thread_data_t;
typedef struct task_data_t task_data_t;
typedef struct region_context_t region_context_t;

/* Parallel region type */
struct parallel_data_t {
    unique_id_t         id;
    task_graph_node_t  *parallel_begin_node_ref;
    task_graph_node_t  *parallel_end_node_ref;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
    region_context_t   *context;
};

/* Thread type */
struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;

    /* Record the sequence of nested regions that led to the current context */
    stack_t            *region_context_stack;

    /* Record a reference to an initial task's graph node for a subsequent
       parallel region */
    task_graph_node_t  *initial_task_graph_node_ref;
};

/* Task type */
struct task_data_t {
    unique_id_t         id;
    task_graph_node_t  *task_node_ref;
    ompt_task_flag_t    type;
    tree_node_t        *tree_node;

    /* only accessed by implicit tasks which are children of an initial task to
       atomically register as children of the initial task. This is because the
       implicit-task-begin event happens in the context of the implicit child
       task rather than in that of the initial parent task
     */
    pthread_mutex_t    *lock;

    unique_id_t         enclosing_parallel_id;

    /* For tasks that encounter a workshare construct, this represents the 
       pseudo-task of the workshare construct. Any tasks generated during the
       workshare construct should be considered children of this pseudo-task
       rather than of the task that encountered the workshare region.

       Only allocated/deallocated in ompt_callback_work.

       Only accessed in task_create.
    */
    task_data_t        *workshare_child_task;

    /* track the context that encloses this task */
    // region_context_t   *context;

};

/* Label the various kinds of context that can be associated with begin/end 
   events
*/
typedef enum {

    context_parallel,

    /* Worksharing Contexts */
    context_sections,
    context_single,

    /* Worksharing-loop Contexts */
    context_loop,
    context_taskloop,

    /* Synchronisation Contexts */

    context_sync_taskgroup,

    /* Standalone (i.e. no nested contexts) synchronisation contexts */
    context_sync_barrier,
    context_sync_barrier_implicit,
    context_sync_barrier_explicit,
    context_sync_barrier_implementation,
    context_sync_taskwait,

    /* not needed for now, but may be later:
        context_distribute,
        context_sync_reduction
    */

} context_t;

struct region_context_t {
    context_t           type;
    void               *context_data;
    stack_t            *context_task_graph_nodes;
    task_graph_node_t  *context_begin_node;
    task_graph_node_t  *context_end_node;
    pthread_mutex_t     lock;
};

#endif // OMPT_CORE_TYPES_H
