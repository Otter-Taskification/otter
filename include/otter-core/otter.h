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
#include <otter-task-graph/task-tree.h>
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

/* ancestor level for innermost parallel region */
#define INNER 0

/* return values from ompt_get_parallel_info_t */
#define PARALLEL_INFO_AVAIL     2
#define PARALLEL_INFO_UNAVAIL   1
#define PARALLEL_INFO_NONE      0

/* Packing task type & parallel region into task ID value

   Have 64 bits available in unique_id_t/tree_node_id_t/array_id_t but never
   going to need all of them for a unique task ID in any realistic scenario

   => use some of the bits to also pass to task-tree a task's type & parallel
        region

   task type:       0xf000000000000000 => 16 values (60-bit shift)
   parallel region: 0x00ff000000000000 => 255 values (48-bit shift)

    __builtin_ctzll - Returns the number of trailing 0-bits in x, starting at 
    the least significant bit position. If x is 0, the result is undefined

    I use this built-in to convert ompt_task_flag_t to an int representing the
    bit that is set i.e. 0x01 -> 0, 0x08 -> 3 etc. This converts a value like
    0b1000 into 0b0011 which requires fewer bits. This means I can represent up
    to 16 task types in the top 4 bits of the task ID, instead of setting 16
    independent bits

    NOTE: need to check whether this is portable between clang & icc

 */
#define PACK_TASK_BITS(flags, task_id, parallel_id)                           \
    (task_id \
        | ( (unique_id_t)__builtin_ctzll(flags) << TASK_TREE_TASK_TYPE_SHFT ) \
        | ((parallel_id & 0xFF)<<TASK_TREE_PARALLEL_ID_SHIFT) )

#endif // OTTER_H
