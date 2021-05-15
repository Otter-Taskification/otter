#if !defined(OTTER_TRACE_H)
#define OTTER_TRACE_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include <otter-ompt-header.h>
#include <otter-common.h>

#define DEFAULT_LOCATION_GRP 0 // OTF2_UNDEFINED_LOCATION_GROUP
#define DEFAULT_SYSTEM_TREE  0
#define DEFAULT_NAME_BUF_SZ  64

/* event endpoint */
typedef enum {
    trace_event_type_enter = ompt_scope_begin,
    trace_event_type_leave = ompt_scope_end,
    trace_event_type_task_create
} trace_event_type_t;

/* opaque types */
typedef struct trace_region_def_t trace_region_def_t;
typedef struct trace_location_def_t trace_location_def_t;

/* interface function prototypes */
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive();

/* Location definition */
trace_location_def_t *trace_new_location_definition(uint64_t id, ompt_thread_t thread_type, OTF2_LocationType loc_type, OTF2_LocationGroupRef loc_grp);

/* Region definitions */
trace_region_def_t *trace_new_parallel_region(unique_id_t id, unique_id_t master, int flags, unsigned int requested_parallelism);
trace_region_def_t *trace_new_workshare_region(trace_location_def_t *loc, ompt_work_t wstype, uint64_t count);
trace_region_def_t *trace_new_sync_region(trace_location_def_t *loc, ompt_sync_region_t stype, unique_id_t encountering_task_id);
trace_region_def_t *trace_new_task_region(trace_location_def_t *loc, trace_region_def_t *parent_task_region, unique_id_t task_id, ompt_task_flag_t flags, int has_dependences);

void trace_destroy_location(trace_location_def_t *loc);
void trace_destroy_parallel_region(trace_region_def_t *rgn);
void trace_destroy_workshare_region(trace_region_def_t *rgn);
void trace_destroy_sync_region(trace_region_def_t *rgn);
void trace_destroy_task_region(trace_region_def_t *rgn);

/* write events */
void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self);
void trace_event_task_create(trace_location_def_t *self, trace_region_def_t *created_task);
void trace_event_task_schedule(trace_location_def_t *self, trace_region_def_t *prior_task, ompt_task_status_t prior_status);
// void trace_event(trace_location_def_t *self, trace_region_def_t *region, trace_event_type_t event_type);
// void trace_event_task_switch(trace_location_def_t *self);
// void trace_event_task_complete(trace_location_def_t *self);




#endif // OTTER_TRACE_H
