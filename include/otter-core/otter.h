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

/* Used as an array index to keep track of unique ids for different entities */
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

unique_id_t get_unique_id(unique_id_type_t id_type);

/* Label the various kinds of scopes that a thread can encounter */
typedef enum {

    scope_unknown,

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

} scope_type_t;

/* ancestor level for innermost parallel region */
#define INNER 0

/* return values from ompt_get_parallel_info_t */
#define PARALLEL_INFO_AVAIL     2
#define PARALLEL_INFO_UNAVAIL   1
#define PARALLEL_INFO_NONE      0

#endif // OTTER_H
