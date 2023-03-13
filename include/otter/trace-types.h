#if !defined(OTTER_TRACE_TYPES_H)
#define OTTER_TRACE_TYPES_H

#include <pthread.h>
#include <otf2/otf2.h>
#include "otter/otter-common.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/otter-task-context.h"

/* Different kinds of unique IDs */
typedef enum trace_ref_type_t {
    trace_region,
    trace_string,
    trace_location,
    trace_other,
    NUM_REF_TYPES // <- MUST BE LAST ENUM ITEM
} trace_ref_type_t;

/* Types of event endpoint */
typedef enum {
    trace_event_type_enter,
    trace_event_type_leave,
    trace_event_type_task_create
} trace_event_type_t;

/* Different kinds of regions supported */
typedef enum {
    trace_region_parallel,
    trace_region_workshare,
    trace_region_synchronise,
    trace_region_task,
#if defined(USE_OMPT_MASKED)
    trace_region_masked,
#else
    trace_region_master,
#endif
    trace_region_phase
} trace_region_type_t;

typedef enum otter_thread_t {
    otter_thread_initial = 1,
    otter_thread_worker  = 2,
    otter_thread_other   = 3,
    otter_thread_unknown = 4
} otter_thread_t;

typedef enum otter_work_t {
    otter_work_loop            = 1,
    otter_work_sections        = 2,
    otter_work_single_executor = 3,
    otter_work_single_other    = 4,
    otter_work_workshare       = 5,
    otter_work_distribute      = 6,
    otter_work_taskloop        = 7
} otter_work_t;

typedef enum otter_sync_region_t {
    otter_sync_region_barrier                = 1,
    otter_sync_region_barrier_implicit       = 2,
    otter_sync_region_barrier_explicit       = 3,
    otter_sync_region_barrier_implementation = 4,
    otter_sync_region_taskwait               = 5,
    otter_sync_region_taskgroup              = 6,
    otter_sync_region_reduction              = 7,
    otter_sync_region_barrier_implicit_workshare = 8,
    otter_sync_region_barrier_implicit_parallel = 9,
    otter_sync_region_barrier_teams = 10
} otter_sync_region_t;

typedef enum otter_phase_region_t {
    otter_phase_region_generic = 1
} otter_phase_region_t;

typedef enum otter_task_flag_t {
    otter_task_initial    = 0x00000001,
    otter_task_implicit   = 0x00000002,
    otter_task_explicit   = 0x00000004,
    otter_task_target     = 0x00000008,
    otter_task_taskwait   = 0x00000010,
    otter_task_undeferred = 0x08000000,
    otter_task_untied     = 0x10000000,
    otter_task_final      = 0x20000000,
    otter_task_mergeable  = 0x40000000,
    otter_task_merged     = 0x80000000
} otter_task_flag_t;

typedef enum otter_task_status_t {
    otter_task_complete      = 1,
    otter_task_yield         = 2,
    otter_task_cancel        = 3,
    otter_task_detach        = 4,
    otter_task_early_fulfill = 5,
    otter_task_late_fulfill  = 6,
    otter_task_switch        = 7,
    otter_taskwait_complete  = 8
} otter_task_status_t;

/* Attributes of a parallel region */
typedef struct {
    unique_id_t     id;
    unique_id_t     master_thread;
    bool            is_league;
    unsigned int    requested_parallelism;
    unsigned int    ref_count;
    unsigned int    enter_count;
    pthread_mutex_t lock_rgn;
    otter_queue_t  *rgn_defs;
} trace_parallel_region_attr_t;

/* Attributes of a workshare region */
typedef struct {
    otter_work_t    type;
    uint64_t        count;
} trace_wshare_region_attr_t;

/* Attributes of a master region */
typedef struct {
    uint64_t        thread;
} trace_master_region_attr_t;

/* Attributes of a sync region */
typedef struct {
    otter_sync_region_t type;
    bool                sync_descendant_tasks;
    unique_id_t         encountering_task_id;
} trace_sync_region_attr_t;

/* Attributes of a task region */
typedef struct {
    unique_id_t         id;
    otter_task_flag_t   type;               
    otter_task_flag_t   flags;              
    int                 has_dependences;
    unique_id_t         parent_id;
    otter_task_flag_t   parent_type;        
    otter_task_status_t task_status;
    otter_string_ref_t  source_file_name_ref;
    otter_string_ref_t  source_func_name_ref;
    int                 source_line_number;
    const void         *task_create_ra;
} trace_task_region_attr_t;

/* Attributes of a phase region */
typedef struct {
    otter_phase_region_t    type;
    otter_string_ref_t      name;
} trace_phase_region_attr_t;

typedef union {
    trace_parallel_region_attr_t    parallel;
    trace_wshare_region_attr_t      wshare;
    trace_master_region_attr_t      master;
    trace_sync_region_attr_t        sync;
    trace_task_region_attr_t        task;
    trace_phase_region_attr_t       phase;
} trace_region_attr_t;

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
typedef struct {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    unique_id_t          encountering_task_id;
    otter_stack_t       *rgn_stack;
    trace_region_attr_t  attr;    
} trace_region_def_t;

#endif // OTTER_TRACE_TYPES_H
