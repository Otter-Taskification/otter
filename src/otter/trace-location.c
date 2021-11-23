#include <stdlib.h>
#include <pthread.h>
#include <otf2/otf2.h>
#include "otter/trace-lookup-macros.h"
#include "otter/trace-location.h"
#include "otter/queue.h"
#include "otter/stack.h"

/* Defined in trace.c */
extern OTF2_Archive *Archive;
// extern OTF2_GlobalDefWriter *Defs;
// extern pthread_mutex_t lock_global_def_writer;
// extern pthread_mutex_t lock_global_archive;

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
