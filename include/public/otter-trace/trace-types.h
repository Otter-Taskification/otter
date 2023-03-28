/**
 * @file trace-types.h
 * @author Adam Tuft
 * @brief Defines various public enums for use by consumers of otter-trace.
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_TYPES_H)
#define OTTER_TRACE_TYPES_H

typedef enum {
    otter_thread_initial = 1,
    otter_thread_worker  = 2,
    otter_thread_other   = 3,
    otter_thread_unknown = 4
} otter_thread_t;

typedef enum {
    otter_work_loop            = 1,
    otter_work_sections        = 2,
    otter_work_single_executor = 3,
    otter_work_single_other    = 4,
    otter_work_workshare       = 5,
    otter_work_distribute      = 6,
    otter_work_taskloop        = 7
} otter_work_t;

typedef enum {
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

typedef enum {
    otter_phase_region_generic = 1
} otter_phase_region_t;

typedef enum {
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

typedef enum {
    otter_task_complete      = 1,
    otter_task_yield         = 2,
    otter_task_cancel        = 3,
    otter_task_detach        = 4,
    otter_task_early_fulfill = 5,
    otter_task_late_fulfill  = 6,
    otter_task_switch        = 7,
    otter_taskwait_complete  = 8
} otter_task_status_t;

#endif // OTTER_TRACE_TYPES_H
