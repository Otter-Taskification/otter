#if !defined(OTTER_TRACE_STRUCTS_H)
#define OTTER_TRACE_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include <otter-ompt-header.h>
#include <otter-common.h>
#include <otter-datatypes/queue.h>
#include <otter-datatypes/stack.h>
#include <otter-trace/trace.h>

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
    queue_t        *rgn_defs;
};

/* Attributes of a workshare region */
struct trace_wshare_region_attr_t {
    ompt_work_t     type;
    uint64_t        count;
};

/* Attributes of a master region */
struct trace_master_region_attr_t {
    uint64_t        thread;
};

/* Attributes of a sync region */
struct trace_sync_region_attr_t {
    ompt_sync_region_t  type;
    unique_id_t         encountering_task_id;
};

/* Attributes of a task region */
struct trace_task_region_attr_t {
    unique_id_t         id;
    ompt_task_flag_t    type;
    ompt_task_flag_t    flags;
    int                 has_dependences;
    unique_id_t         parent_id;
    ompt_task_flag_t    parent_type;
    ompt_task_status_t  task_status;
};

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
struct trace_region_def_t {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    unique_id_t          encountering_task_id;
    union {
        trace_parallel_region_attr_t    parallel;
        trace_wshare_region_attr_t      wshare;
        trace_master_region_attr_t      master;
        trace_sync_region_attr_t        sync;
        trace_task_region_attr_t        task;
    } attr;
};

/* Store values needed to register location definition (threads) with OTF2 */
struct trace_location_def_t {
    unique_id_t             id;
    ompt_thread_t           thread_type;
    uint64_t                events;
    stack_t                *rgn_stack;
    queue_t                *rgn_defs;
    stack_t                *rgn_defs_stack;
    OTF2_LocationRef        ref;
    OTF2_LocationType       type;
    OTF2_LocationGroupRef   location_group;
    OTF2_AttributeList     *attributes;
    OTF2_EvtWriter         *evt_writer;
    OTF2_DefWriter         *def_writer;
};

/* Create new location */
trace_location_def_t *
trace_new_location_definition(
    uint64_t              id,
    ompt_thread_t         thread_type,
    OTF2_LocationType     loc_type, 
    OTF2_LocationGroupRef loc_grp);

/* Create new region */
trace_region_def_t *
trace_new_parallel_region(
    unique_id_t    id, 
    unique_id_t    master,
    unique_id_t    encountering_task_id,
    int            flags,
    unsigned int   requested_parallelism);

trace_region_def_t *
trace_new_workshare_region(
    trace_location_def_t *loc,
    ompt_work_t           wstype,
    uint64_t              count,
    unique_id_t           encountering_task_id);

trace_region_def_t *
trace_new_master_region(
    trace_location_def_t *loc,
    unique_id_t           encountering_task_id);

trace_region_def_t *
trace_new_sync_region(
    trace_location_def_t *loc,
    ompt_sync_region_t    stype,
    unique_id_t           encountering_task_id);

trace_region_def_t *
trace_new_task_region(
    trace_location_def_t *loc,
    trace_region_def_t   *parent_task_region,
    unique_id_t           task_id,
    ompt_task_flag_t      flags,
    int                   has_dependences);

/* Destroy location/region */
void trace_destroy_location(trace_location_def_t *loc);
void trace_destroy_parallel_region(trace_region_def_t *rgn);
void trace_destroy_workshare_region(trace_region_def_t *rgn);
void trace_destroy_master_region(trace_region_def_t *rgn);
void trace_destroy_sync_region(trace_region_def_t *rgn);
void trace_destroy_task_region(trace_region_def_t *rgn);

/* pretty-print region definitions */
void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_STRUCTS_H
