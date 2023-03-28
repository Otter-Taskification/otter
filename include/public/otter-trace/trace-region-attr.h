/**
 * @file trace-region-attr.h
 * @author Adam Tuft
 * @brief Defines the public attributes of various kinds of regions.
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"

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
