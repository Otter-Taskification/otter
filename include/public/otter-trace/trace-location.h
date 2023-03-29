/**
 * @file trace-location.c
 * @author Adam Tuft
 * @brief Defines trace_location_def_t which represents an OTF2 location, used
 * to record the location's definition in the trace. Responsible for new/delete,
 * adding a thread's attributes to its OTF2 attribute list when recording an 
 * event, and writing a location's definition to the trace.
 */

#if !defined(OTTER_TRACE_LOCATION_H)
#define OTTER_TRACE_LOCATION_H

#include <otf2/otf2.h>
#include "public/otter-common.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace-types.h"

// Represents a location definition of an OTF2 trace
typedef struct trace_location_def_t trace_location_def_t;

/* Create new location */
trace_location_def_t *trace_new_location_definition(
    uint64_t              id,
    otter_thread_t        thread_type,
    OTF2_LocationType     loc_type, 
    OTF2_LocationGroupRef loc_grp
);
void trace_destroy_location(trace_location_def_t *loc);
void trace_add_thread_attributes(trace_location_def_t *self);
void trace_write_location_definition(trace_location_def_t *loc);
bool trace_location_pop_region_def(trace_location_def_t *loc, data_item_t *dest);
bool trace_location_push_region_def(trace_location_def_t *loc, data_item_t item);
size_t trace_location_get_num_region_def(trace_location_def_t *loc);
unique_id_t trace_location_get_id(trace_location_def_t *loc);

#endif // OTTER_TRACE_LOCATION_H
