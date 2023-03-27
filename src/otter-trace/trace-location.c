/**
 * @file trace-location.c
 * @author Adam Tuft
 * @brief Defines trace_location_def_t which represents an OTF2 location, used
 * to record the location's definition in the trace. Responsible for new/delete,
 * adding a thread's attributes to its OTF2 attribute list when recording an 
 * event, and writing a location's definition to the trace.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <otf2/otf2.h>
#include "private/otter-trace/trace-lookup-macros.h"
#include "src/otter-trace/trace-attributes.h"
#include "public/otter-trace/trace-location.h"
#include "src/otter-trace/trace-archive.h"
#include "private/otter-trace/trace-unique-refs.h"
#include "private/otter-trace/trace-check-error-code.h"
#include "private/otter-trace/trace-static-constants.h"
#include "public/types/queue.h"
#include "public/types/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

trace_location_def_t *
trace_new_location_definition(
    unique_id_t            id,
    otter_thread_t         thread_type,
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

    OTF2_Archive *Archive = get_global_archive();

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
    OTF2_AttributeList_Delete(loc->attributes);
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
        self->thread_type == otter_thread_initial ? 
            attr_label_ref[attr_thread_type_initial] :
        self->thread_type == otter_thread_worker ? 
            attr_label_ref[attr_thread_type_worker] : 0);
    CHECK_OTF2_ERROR_CODE(r);
    return;
}

void
trace_write_location_definition(trace_location_def_t *loc)
{
    if (loc == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }

    char location_name[DEFAULT_NAME_BUF_SZ + 1] = {0};
    OTF2_StringRef location_name_ref = get_unique_str_ref();
    snprintf(location_name, DEFAULT_NAME_BUF_SZ, "Thread %lu", loc->id);

    LOG_DEBUG("[t=%lu] locking global def writer", loc->id);
    pthread_mutex_t *def_writer_lock = global_def_writer_lock();
    pthread_mutex_lock(def_writer_lock);

    OTF2_GlobalDefWriter *Defs = get_global_def_writer();
    OTF2_GlobalDefWriter_WriteString(Defs,
        location_name_ref,
        location_name);

    LOG_DEBUG("[t=%lu] writing location definition", loc->id);
    OTF2_GlobalDefWriter_WriteLocation(Defs,
        loc->ref,
        location_name_ref,
        loc->type,
        loc->events,
        loc->location_group);

    LOG_DEBUG("[t=%lu] unlocking global def writer", loc->id);
    pthread_mutex_unlock(def_writer_lock);

    return;
}
