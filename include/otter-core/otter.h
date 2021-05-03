#if !defined(OTTER_H)
#define OTTER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

// getrusage
#include <sys/time.h>
#include <sys/resource.h>

#include <otter-ompt-header.h>

#include <otter-common.h>
#include <otter-core/otter-entry.h>
#include <otter-datatypes/stack.h>
#include <otter-task-graph/task-graph.h>
#include <otter-trace/trace.h>

/* A naming convention was chosen for the callbacks so that each callback is 
   named after the event in ompt.h which it handles. The struct passed to the
   tool by ompt-core has a member for each OMP event. This macro attaches
   a callback to its corresponding field in that struct.
*/
#define include_callback(callbacks, event) callbacks->on_##event = on_##event

#define STR_EQUAL(a, b) (!strcmp(a,b))

/* Include the function prototypes for the callbacks this tool implements */
#define implements_callback_thread_begin   
#define implements_callback_thread_end     
#define implements_callback_parallel_begin 
#define implements_callback_parallel_end   
#define implements_callback_task_create    
#define implements_callback_task_schedule  
#define implements_callback_implicit_task
#define implements_callback_work
#define implements_callback_sync_region
#include <otter-core/ompt-callback-prototypes.h>

/* Used as an array index to keep track of unique id's for different entities */
typedef enum unique_id_type_t {
    id_timestamp        ,
    id_parallel         ,
    id_thread           ,
    id_task             ,
    NUM_ID_TYPES
} unique_id_type_t;
#define get_unique_parallel_id() get_unique_id(id_parallel)
#define get_unique_thread_id()   get_unique_id(id_thread)
#define get_unique_task_id()     get_unique_id(id_task)
#define get_dummy_time()         get_unique_id(id_timestamp)

/* forward declarations */
typedef struct parallel_data_t parallel_data_t;
typedef struct thread_data_t thread_data_t;
typedef struct task_data_t task_data_t;
typedef struct region_scope_t region_scope_t;

/* Parallel region type */
struct parallel_data_t {
    unique_id_t         id;
    int                 flags;
    unsigned int        actual_parallelism;
    task_graph_node_t  *parallel_begin_node_ref;
    task_graph_node_t  *parallel_end_node_ref;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
    region_scope_t     *scope;
};

/* Thread type */
struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;

    /* Record the sequence of nested regions that led to the current scope */
    stack_t            *region_scope_stack;

    /* Scope most recently popped from OR pushed to the region_scope_stack */
    region_scope_t     *prior_scope;

    /* Record a reference to an initial task's graph node for a subsequent
       parallel region */
    task_graph_node_t  *initial_task_graph_node_ref;

    bool                is_master_thread; // of current parallel region
};

/* Task type */
struct task_data_t {
    unique_id_t         id;
    task_graph_node_t  *task_node_ref;
    ompt_task_flag_t    type;
    ompt_task_flag_t    flags;

    /* only accessed by implicit tasks which are children of an initial task to
       atomically register as children of the initial task. This is because the
       implicit-task-begin event happens in the scope of the implicit child
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

    /* track the scope that encloses this task */
    // region_scope_t   *scope;

};

/* Label the various kinds of scopes that a thread can encounter */
typedef enum {

    scope_parallel,

    /* Worksharing scopes */
    scope_sections,
    scope_single,

    /* Worksharing-loop scopes */
    scope_loop,
    scope_taskloop,

    /* Synchronisation scopes */

    scope_sync_taskgroup,

    /* Standalone (i.e. no nested scopes) synchronisation scopes */
    scope_sync_barrier,
    scope_sync_barrier_implicit,
    scope_sync_barrier_explicit,
    scope_sync_barrier_implementation,
    scope_sync_taskwait,

    /* not needed for now, but may be later:
        scope_distribute,
        scope_sync_reduction
    */

} scope_t;

struct region_scope_t {
    scope_t                 type;
    ompt_scope_endpoint_t   endpoint;
    void                   *data;
    stack_t                *task_graph_nodes;
    task_graph_node_t      *begin_node;
    task_graph_node_t      *end_node;
    pthread_mutex_t         lock;
};

/* ancestor level for innermost parallel region */
#define INNER 0

/* return values from ompt_get_parallel_info_t */
#define PARALLEL_INFO_AVAIL     2
#define PARALLEL_INFO_UNAVAIL   1
#define PARALLEL_INFO_NONE      0

#endif // OTTER_H
