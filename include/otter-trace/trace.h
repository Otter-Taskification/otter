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

/* Location definition */
trace_location_def_t *trace_new_location_definition(
    uint64_t id, ompt_thread_t thread_type, 
    OTF2_LocationType loc_type, OTF2_LocationGroupRef loc_grp);

/* Parallel region is written to global def writer */
trace_region_def_t *trace_new_parallel_region(unique_id_t id, unique_id_t master, int flags, unsigned int requested_parallelism);
void trace_destroy_parallel_region(trace_region_def_t *rgn);

/* Other region defs are written to local def writers (via self->def_writer) */
trace_region_def_t *trace_new_workshare_region(trace_location_def_t *self, ompt_work_t wstype, uint64_t count);
trace_region_def_t *trace_new_sync_region(trace_location_def_t *self, ompt_sync_region_t stype, unique_id_t encountering_task_id);

/* write events */
void trace_event_thread(trace_location_def_t *self, ompt_scope_endpoint_t endpoint);
void trace_event(trace_location_def_t *self, trace_region_def_t *region, ompt_scope_endpoint_t endpoint);
// void trace_event_parallel(trace_location_def_t *self, trace_region_def_t *region, ompt_scope_endpoint_t endpoint);
// void trace_event_workshare(trace_location_def_t *self, trace_region_def_t *region, ompt_scope_endpoint_t endpoint);
// void trace_event_synchronise(trace_location_def_t *self, trace_region_def_t *region, ompt_scope_endpoint_t endpoint);
// void trace_event_task_create( trace_location_def_t *self,
//     uint32_t creating_thread, ompt_task_flag_t flags);
// void trace_event_task_switch(trace_location_def_t *self);
// void trace_event_task_complete(trace_location_def_t *self);

/* getters */
uint64_t trace_get_location_events(trace_location_def_t *self);

#endif // OTTER_TRACE_H
