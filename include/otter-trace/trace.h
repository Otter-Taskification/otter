#if !defined(OTTER_TRACE_H)
#define OTTER_TRACE_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include <otter-ompt-header.h>
#include <otter-common.h>

#define DEFAULT_LOCATION_GRP 0
#define DEFAULT_SYSTEM_TREE  0
#define DEFAULT_COMM_REF     0
#define DEFAULT_NAME_BUF_SZ  64

/* opaque types */
typedef struct trace_region_def_t trace_region_def_t;
typedef struct trace_location_def_t trace_location_def_t;

/* interface function prototypes */
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive();

/* new definitions */
trace_location_def_t *trace_new_location_definition(
    uint64_t id, OTF2_LocationType loc_type, OTF2_LocationGroupRef loc_grp);
trace_region_def_t *trace_new_region_definition(
    uint64_t id, OTF2_RegionRole rgn_role);

/* write event */
void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self, trace_region_def_t *region);
// void trace_event_thread_fork(trace_location_def_t *self, uint64_t requested);
// void trace_event_thread_create(trace_location_def_t *self, uint64_t child_id);
// void trace_event_thread_join(trace_location_def_t *self);
// void trace_event_task_create( trace_location_def_t *self,
//     uint32_t creating_thread, ompt_task_flag_t flags);
// void trace_event_task_switch(trace_location_def_t *self);
// void trace_event_task_complete(trace_location_def_t *self);

/* getters */
uint64_t trace_get_location_events(trace_location_def_t *self);

#endif // OTTER_TRACE_H
