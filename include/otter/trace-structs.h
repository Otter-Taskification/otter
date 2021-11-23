#if !defined(OTTER_TRACE_STRUCTS_H)
#define OTTER_TRACE_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <otf2/otf2.h>

#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/trace.h"
#include "otter/trace-location.h"
#include "otter/trace-region-parallel.h"
#include "otter/trace-region-workshare.h"
#include "otter/trace-region-master.h"
#include "otter/trace-region-sync.h"
#include "otter/trace-region-task.h"

/* Forward definitions */
typedef struct trace_parallel_region_attr_t trace_parallel_region_attr_t;
typedef struct trace_region_attr_empty_t    trace_region_attr_empty_t;
typedef struct trace_wshare_region_attr_t   trace_wshare_region_attr_t;
typedef struct trace_master_region_attr_t   trace_master_region_attr_t;
typedef struct trace_sync_region_attr_t     trace_sync_region_attr_t;
typedef struct trace_task_region_attr_t     trace_task_region_attr_t;

/* Attributes of a parallel region */
struct trace_parallel_region_attr_t {
    unique_id_t     id;
    unique_id_t     master_thread;
    bool            is_league;
    unsigned int    requested_parallelism;
    unsigned int    ref_count;
    unsigned int    enter_count;
    pthread_mutex_t lock_rgn;
    otter_queue_t        *rgn_defs;
};

/* Attributes of a workshare region */
struct trace_wshare_region_attr_t {
    ompt_work_t     type;       // TODO: decouple
    uint64_t        count;
};

/* Attributes of a master region */
struct trace_master_region_attr_t {
    uint64_t        thread;
};

/* Attributes of a sync region */
struct trace_sync_region_attr_t {
    ompt_sync_region_t  type;       // TODO: decouple
    unique_id_t         encountering_task_id;
};

/* Attributes of a task region */
struct trace_task_region_attr_t {
    unique_id_t         id;
    ompt_task_flag_t    type;               // TODO: decouple
    ompt_task_flag_t    flags;              // TODO: decouple
    int                 has_dependences;
    unique_id_t         parent_id;
    ompt_task_flag_t    parent_type;        // TODO: decouple
    ompt_task_status_t  task_status;        // TODO: decouple
};

typedef union {
    trace_parallel_region_attr_t    parallel;
    trace_wshare_region_attr_t      wshare;
    trace_master_region_attr_t      master;
    trace_sync_region_attr_t        sync;
    trace_task_region_attr_t        task;
} trace_region_attr_t;

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
struct trace_region_def_t {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    unique_id_t          encountering_task_id;
    otter_stack_t       *rgn_stack;
    trace_region_attr_t  attr;    
};

/* pretty-print region definitions */
void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_STRUCTS_H
