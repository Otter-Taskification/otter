#if !defined(OTTER_TRACE_OMPT_H)
#define OTTER_TRACE_OMPT_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-def.h"

// TODO: these declarations should be exposed in public/otter-trace/trace-archive.h
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

// TODO: move these declarations to their own header under public/otter-trace
void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self);
void trace_event_task_create(trace_location_def_t *self, trace_region_def_t *created_task);
void trace_event_task_schedule(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status);
void trace_event_task_switch(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status, trace_region_def_t *next_task);
void trace_write_region_definition(trace_region_def_t *rgn);

#endif // OTTER_TRACE_OMPT_H
