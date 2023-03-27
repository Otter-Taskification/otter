#if !defined(OTTER_TRACE_H)
#define OTTER_TRACE_H

// TODO: too much functionality is coupled together in this interface to otter-trace!!!
// TODO: refactor file to only expose the subset of functionality needed
// TODO: not all consumers of otter-trace use region definitions, for example.

#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-parallel.h"
#include "public/otter-trace/trace-region-task.h"
#include "public/otter-trace/trace-region-workshare.h"
#include "public/otter-trace/trace-region-master.h"
#include "public/otter-trace/trace-region-sync.h"
#include "public/otter-trace/trace-region-phase.h"

/**************************/

#include "public/otter-common.h"



/**
 * @brief Defines whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 * Where they are exposed to the user, a module should wrap this in its own enum
 * type so as not to expose the internal interface between otter modules.
 * 
 */
typedef enum {
    trace_sync_children,
    trace_sync_descendants
} trace_task_sync_t;


bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

/* Functions defined in trace-core.c */
void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self);
void trace_event_task_create(trace_location_def_t *self, trace_region_def_t *created_task);
void trace_event_task_schedule(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status);
void trace_event_task_switch(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status, trace_region_def_t *next_task);
// void trace_event_task_complete(trace_location_def_t *self);
void trace_write_region_definition(trace_region_def_t *rgn);

/* Functions defined in trace-task-graph.c */
void trace_graph_event_task_begin(otter_task_context *task, trace_region_attr_t task_attr);
void trace_graph_event_task_end(otter_task_context *task);
void trace_graph_synchronise_tasks(otter_task_context *task, trace_region_attr_t sync_attr);

void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_H
