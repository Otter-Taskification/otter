/**
 * @file trace-location.c
 * @author Adam Tuft
 * @brief Defines trace_location_def_t which represents an OTF2 location, used
 * to record the location's definition in the trace. Responsible for new/delete,
 * adding a thread's attributes to its OTF2 attribute list when recording an
 * event, and writing a location's definition to the trace.
 */

#pragma once

#if !defined(OTTER_TRACE_LOCATION_H)
#define OTTER_TRACE_LOCATION_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-region-def.h"
#include "public/otter-trace/trace-types.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include <otf2/OTF2_DefWriter.h>
#include <otf2/OTF2_Definitions.h>
#include <otf2/OTF2_EvtWriter.h>

// Represents a location definition of an OTF2 trace
typedef struct trace_location_def_t trace_location_def_t;

/* Create new location */
trace_location_def_t *
trace_new_location_definition(uint64_t id, otter_thread_t thread_type,
                              OTF2_LocationType loc_type,
                              OTF2_LocationGroupRef loc_grp);
void trace_destroy_location(trace_location_def_t *loc);
void trace_write_location_definition(trace_location_def_t *loc);
bool trace_location_get_region_def(trace_location_def_t *loc,
                                   trace_region_def_t **rgn);
bool trace_location_store_region_def(trace_location_def_t *loc,
                                     trace_region_def_t *rgn);
size_t trace_location_get_num_region_def(trace_location_def_t *loc);
unique_id_t trace_location_get_id(trace_location_def_t *loc);
otter_thread_t trace_location_get_thread_type(trace_location_def_t *loc);
void trace_location_get_otf2(trace_location_def_t *loc,
                             OTF2_AttributeList **attributes,
                             OTF2_EvtWriter **evt_writer,
                             OTF2_DefWriter **def_writer);
void trace_location_inc_event_count(trace_location_def_t *loc);
void trace_location_enter_region_def_scope(trace_location_def_t *loc);
void trace_location_leave_region_def_scope(trace_location_def_t *loc,
                                           trace_region_def_t *rgn);
void trace_location_enter_region(trace_location_def_t *loc,
                                 trace_region_def_t *rgn);
void trace_location_leave_region(trace_location_def_t *loc,
                                 trace_region_def_t **rgn);
void trace_location_get_active_regions_from_task(trace_location_def_t *loc,
                                                 trace_region_def_t *task);
void trace_location_store_active_regions_in_task(trace_location_def_t *loc,
                                                 trace_region_def_t *task);

#endif // OTTER_TRACE_LOCATION_H
