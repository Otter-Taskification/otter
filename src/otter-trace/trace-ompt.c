#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <otf2/otf2.h>

#include "public/debug.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-ompt.h"
#include "public/otter-trace/trace-location.h"

#include "otter-trace/trace-timestamp.h"
#include "otter-trace/trace-attributes.h"
#include "otter-trace/trace-archive-impl.h"
#include "otter-trace/trace-types-as-labels.h"
#include "otter-trace/trace-unique-refs.h"
#include "otter-trace/trace-check-error-code.h"
#include "otter-trace/trace-static-constants.h"
#include "otter-trace/trace-attribute-lookup.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE EVENTS                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
trace_event_thread_begin(trace_location_def_t *self)
{
    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    OTF2_DefWriter *def_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, &def_writer);
    unique_id_t thread_id = trace_location_get_id(self);
    otter_thread_t thread_type = trace_location_get_thread_type(self);

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_unique_id,
        thread_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_thread_type,
        attr_label_ref[thread_type_as_label(thread_type)]
    );
    CHECK_OTF2_ERROR_CODE(err);
    
    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_begin]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_EvtWriter_ThreadBegin(
        evt_writer,
        attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        thread_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    trace_location_inc_event_count(self);

    return;
}

void
trace_event_thread_end(trace_location_def_t *self)
{
    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    OTF2_DefWriter *def_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, &def_writer);
    unique_id_t thread_id = trace_location_get_id(self);
    otter_thread_t thread_type = trace_location_get_thread_type(self);

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_unique_id,
        thread_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_thread_type,
        attr_label_ref[thread_type_as_label(thread_type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_end]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_EvtWriter_ThreadEnd(
        evt_writer,    
        attributes, 
        get_timestamp(), 
        OTF2_UNDEFINED_COMM, 
        thread_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    trace_location_inc_event_count(self);

    return;
}

void
trace_event_enter(
    trace_location_def_t *self,
    trace_region_def_t *region)
{
    LOG_ERROR_IF((region == NULL), "null region pointer");

    OTF2_ErrorCode err = OTF2_SUCCESS;
    attr_label_enum_t event_type_label = 0;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, NULL);

    if (trace_region_is_type(region, trace_region_parallel)) {
        trace_location_enter_region_def_scope(self);
    }

    if (trace_region_is_shared(region)) {
        trace_region_lock(region);
    }

    trace_region_attr_t attr = trace_region_get_attributes(region);
    trace_region_type_t region_type = trace_region_get_type(region);
    unique_id_t encountering_task_id = trace_region_get_encountering_task_id(region);

    switch (region_type) {
        case trace_region_parallel:    event_type_label = attr_event_type_parallel_begin;  break;
        case trace_region_workshare:   event_type_label = attr_event_type_workshare_begin; break;
        case trace_region_synchronise: event_type_label = attr_event_type_sync_begin;      break;
        case trace_region_task:        event_type_label = attr_event_type_task_enter;      break;
        case trace_region_master:      event_type_label = attr_event_type_master_begin;    break;
        case trace_region_phase:       event_type_label = attr_event_type_phase_begin;     break;
    }

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_encountering_task_id,
        encountering_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_region_type,
        attr_label_ref[region_type_as_label(region_type, attr)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[event_type_label]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    CHECK_OTF2_ERROR_CODE(err);

    trace_add_region_type_attributes(region, attributes);

    /* Record the event */
    OTF2_EvtWriter_Enter(
        evt_writer,
        attributes,
        get_timestamp(),
        trace_region_get_ref(region)
    );

    trace_location_enter_region(self, region);

    if (trace_region_is_shared(region))
    {
        trace_region_inc_ref_count(region);
        trace_region_unlock(region);
    }

    trace_location_inc_event_count(self);
    return;
}

void
trace_event_leave(trace_location_def_t *self)
{
    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, NULL);

    trace_region_def_t *region = NULL;
    trace_location_leave_region(self, &region);

    if (trace_region_is_shared(region)) {
        trace_region_lock(region);
    }

    attr_label_enum_t event_type_label = 0;
    trace_region_type_t region_type = trace_region_get_type(region);
    unique_id_t encountering_task_id = trace_region_get_encountering_task_id(region);
    trace_region_attr_t attr = trace_region_get_attributes(region);
    
    switch (region_type) {
        case trace_region_parallel:    event_type_label = attr_event_type_parallel_end;  break;
        case trace_region_workshare:   event_type_label = attr_event_type_workshare_end; break;
        case trace_region_synchronise: event_type_label = attr_event_type_sync_end;      break;
        case trace_region_task:        event_type_label = attr_event_type_task_leave;    break;
        case trace_region_master:      event_type_label = attr_event_type_master_end;    break;
        case trace_region_phase:       event_type_label = attr_event_type_phase_end;     break;
    }

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_encountering_task_id,
        encountering_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_region_type,
        attr_label_ref[region_type_as_label(region_type, attr)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[event_type_label]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );
    CHECK_OTF2_ERROR_CODE(err);

    trace_add_region_type_attributes(region, attributes);

    /* Record the event */
    OTF2_EvtWriter_Leave(
        evt_writer,
        attributes,
        get_timestamp(),
        trace_region_get_ref(region)
    );

    if (trace_region_is_type(region, trace_region_parallel)) {
        trace_location_leave_region_def_scope(self, region);
    }
    
    /* Parallel regions must be cleaned up by the last thread to leave */
    if (trace_region_is_shared(region))
    {
        trace_region_dec_ref_count(region);
        // NOTE: must test ref_count == 0 *before* unlocking to avoid races.
        if (trace_region_get_shared_ref_count(region) == 0)
        {
            trace_region_unlock(region);
            trace_destroy_parallel_region(region);
        } else {
            trace_region_unlock(region);
        }
    }

    trace_location_inc_event_count(self);
    
    return;
}

void
trace_event_task_create(
    trace_location_def_t *self, 
    trace_region_def_t   *region)
{
    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, NULL);

    unique_id_t encountering_task_id = trace_region_get_encountering_task_id(region);
    trace_region_type_t region_type = trace_region_get_type(region);
    trace_region_attr_t attr = trace_region_get_attributes(region);
    // https://releases.llvm.org/15.0.0/tools/clang/docs/ReleaseNotes.html#improvements-to-clang-s-diagnostics
    // The -Wint-conversion warning diagnostic for implicit int <-> pointer conversions now defaults to an error in all C language modes.
    uint64_t task_create_ra = (uint64_t) attr.task.task_create_ra;

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_encountering_task_id,
        encountering_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_region_type,
        attr_label_ref[region_type_as_label(region_type, attr)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_task_create]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_task_create_ra,
        task_create_ra
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_unique_id,
        attr.task.id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_task_type,
        attr_label_ref[task_type_as_label(attr.task.type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint32(
        attributes,
        attr_task_flags,
        attr.task.flags
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_parent_task_id,
        attr.task.parent_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_parent_task_type,
        attr_label_ref[task_type_as_label(attr.task.parent_type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_has_dependences,
        attr.task.has_dependences
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_is_undeferred,
        attr.task.flags & otter_task_undeferred ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_is_untied,
        attr.task.flags & otter_task_untied ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_is_final,
        attr.task.flags & otter_task_final ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_is_mergeable,
        attr.task.flags & otter_task_mergeable ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attributes,
        attr_task_is_merged,
        attr.task.flags & otter_task_merged ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_prior_task_status,
        attr_label_ref[task_status_as_label(attr.task.task_status)]
    );
    CHECK_OTF2_ERROR_CODE(err);
    
    OTF2_EvtWriter_ThreadTaskCreate(
        evt_writer, 
        attributes, 
        get_timestamp(), 
        OTF2_UNDEFINED_COMM, 
        OTF2_UNDEFINED_UINT32, 0
    );
    
    trace_location_inc_event_count(self);

    return;
}

void 
trace_event_task_schedule(
    trace_location_def_t    *self,
    trace_region_def_t      *prior_task,
    otter_task_status_t      prior_status)
{
    /* Update prior task's status before recording task enter/leave events */
    LOG_ERROR_IF((trace_region_get_type(prior_task) != trace_region_task), "invalid region type %d", trace_region_get_type(prior_task));
    trace_region_set_task_status(prior_task, prior_status);
    return;
}

void
trace_event_task_switch(
    trace_location_def_t *self, 
    trace_region_def_t   *prior_task, 
    otter_task_status_t   prior_status, 
    trace_region_def_t   *next_task)
{
    // Update prior task's status
    // Transfer thread's active region stack to prior_task->rgn_stack
    // Transfer next_task->rgn_stack to thread
    // Record event with details of tasks swapped & prior_status
    
    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, NULL);

    trace_region_set_task_status(prior_task, prior_status);

    trace_location_store_active_regions_in_task(self, prior_task);
    trace_location_get_active_regions_from_task(self, next_task);

    trace_region_type_t prior_task_region_type = trace_region_get_type(prior_task);
    trace_region_attr_t prior_task_attr = trace_region_get_attributes(prior_task);
    trace_region_attr_t next_task_attr = trace_region_get_attributes(next_task);

    err = OTF2_AttributeList_AddInt32(
        attributes,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_encountering_task_id,
        prior_task_attr.task.id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_region_type,
        attr_label_ref[region_type_as_label(prior_task_region_type, prior_task_attr)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_prior_task_status,
        attr_label_ref[task_status_as_label(prior_status)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_prior_task_id,
        prior_task_attr.task.id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_unique_id,
        next_task_attr.task.id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attributes,
        attr_next_task_id,
        next_task_attr.task.id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_next_task_region_type,
        attr_label_ref[task_type_as_label(next_task_attr.task.type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );
    CHECK_OTF2_ERROR_CODE(err);

    OTF2_EvtWriter_ThreadTaskSwitch(
        evt_writer,
        attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    return;
}
