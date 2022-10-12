#if !defined(OTTER_TRACE_H)
#define OTTER_TRACE_H

#include "otter/trace-enum-types.h"
#include "otter/trace-types.h"
#include "otter/trace-static-constants.h"
#include "otter/trace-location.h"
#include "otter/trace-archive.h"
#include "otter/trace-region-parallel.h"
#include "otter/trace-region-task.h"
#include "otter/trace-region-workshare.h"
#include "otter/trace-region-master.h"
#include "otter/trace-region-sync.h"
#include "otter/trace-region-phase.h"

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
void trace_graph_event_task_begin(otter_task_context *task);
void trace_graph_event_task_end(otter_task_context *task);

void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_H
