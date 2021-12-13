#if !defined(OTTER_TRACE_LOCATION_H)
#define OTTER_TRACE_LOCATION_H

// #include <stdint.h>
// #include <stdbool.h>
#include <otf2/otf2.h>

// #include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/trace-types.h"

/* Store values needed to register location definition (threads) with OTF2 */
typedef struct {
    unique_id_t             id;
    otter_thread_t          thread_type;
    uint64_t                events;
    otter_stack_t          *rgn_stack;
    otter_queue_t          *rgn_defs;
    otter_stack_t          *rgn_defs_stack;
    OTF2_LocationRef        ref;
    OTF2_LocationType       type;
    OTF2_LocationGroupRef   location_group;
    OTF2_AttributeList     *attributes;
    OTF2_EvtWriter         *evt_writer;
    OTF2_DefWriter         *def_writer;
} trace_location_def_t;

/* Create new location */
trace_location_def_t *
trace_new_location_definition(
    uint64_t              id,
    otter_thread_t        thread_type,
    OTF2_LocationType     loc_type, 
    OTF2_LocationGroupRef loc_grp
);

/* Destroy location */
void trace_destroy_location(trace_location_def_t *loc);

void trace_add_thread_attributes(trace_location_def_t *self);

void trace_write_location_definition(trace_location_def_t *loc);

#endif // OTTER_TRACE_LOCATION_H
