#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <otf2/otf2.h>
#include "otter/trace-lookup-macros.h"
#include "otter/trace-attributes.h"
#include "otter/trace-location.h"
#include "otter/queue.h"
#include "otter/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

/* Defined in trace.c */
extern OTF2_Archive *Archive;

trace_location_def_t *
trace_new_location_definition(
    unique_id_t            id,
    ompt_thread_t          thread_type,
    OTF2_LocationType      loc_type,
    OTF2_LocationGroupRef  loc_grp)
{
    trace_location_def_t *new = malloc(sizeof(*new));

    *new = (trace_location_def_t) {
        .id             = id,
        .thread_type    = thread_type,
        .events         = 0,
        .ref            = get_unique_loc_ref(),
        .type           = loc_type,
        .location_group = loc_grp,
        .rgn_stack      = stack_create(),
        .rgn_defs       = queue_create(),
        .rgn_defs_stack = stack_create(),
        .attributes     = OTF2_AttributeList_New()
    };

    new->evt_writer = OTF2_Archive_GetEvtWriter(Archive, new->ref);
    new->def_writer = OTF2_Archive_GetDefWriter(Archive, new->ref);

    /* Thread location definition is written at thread-end (once all events
       counted) */

    LOG_DEBUG("[t=%lu] location created", id);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_stack:",      new->rgn_stack);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_defs:",       new->rgn_defs);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_defs_stack:", new->rgn_defs_stack);

    return new;
}

void 
trace_destroy_location(trace_location_def_t *loc)
{
    if (loc == NULL) return;
    trace_write_location_definition(loc);
    LOG_DEBUG("[t=%lu] destroying rgn_stack %p", loc->id, loc->rgn_stack);
    stack_destroy(loc->rgn_stack, false, NULL);
    if (loc->rgn_defs)
    {
        LOG_DEBUG("[t=%lu] destroying rgn_defs %p", loc->id, loc->rgn_defs);
        queue_destroy(loc->rgn_defs, false, NULL);
    }
    LOG_DEBUG("[t=%lu] destroying rgn_defs_stack %p", loc->id, loc->rgn_defs_stack);
    stack_destroy(loc->rgn_defs_stack, false, NULL);
    // OTF2_AttributeList_Delete(loc->attributes);
    LOG_DEBUG("[t=%lu] destroying location", loc->id);
    free(loc);
    return;
}

void
trace_add_thread_attributes(trace_location_def_t *self)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddInt32(self->attributes, attr_cpu, sched_getcpu());
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint64(self->attributes, attr_unique_id, self->id);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(self->attributes, attr_thread_type,
        self->thread_type == ompt_thread_initial ? 
            attr_label_ref[attr_thread_type_initial] :
        self->thread_type == ompt_thread_worker ? 
            attr_label_ref[attr_thread_type_worker] : 0);
    CHECK_OTF2_ERROR_CODE(r);
    return;
}