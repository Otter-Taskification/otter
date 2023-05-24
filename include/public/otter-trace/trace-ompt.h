/**
 * @file trace-ompt.h
 * @author Adam Tuft
 * @brief Ties together the includes for the components of otter-trace needed by
 * the otter-ompt and otter-serial event sources
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_OMPT_H)
#define OTTER_TRACE_OMPT_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-def.h"
#include "public/otter-trace/trace-initialise.h"

void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self);
void trace_event_task_create(trace_location_def_t *self, trace_region_def_t *created_task);
void trace_event_task_schedule(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status);
void trace_event_task_switch(trace_location_def_t *self, trace_region_def_t *prior_task, otter_task_status_t prior_status, trace_region_def_t *next_task);
// void trace_write_region_definition(trace_region_def_t *rgn);

#endif // OTTER_TRACE_OMPT_H
