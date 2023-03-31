// TODO: this file contains events for the OMPT event model as well as functions for adding region attributes to an attribute list and writing a region definition to a trace. Probably want to decouple these.

// TODO: rename file trace-core.c -> trace-ompt.c
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
#include "otter-trace/trace-archive.h"
#include "otter-trace/trace-lookup-macros.h"
#include "otter-trace/trace-unique-refs.h"
#include "otter-trace/trace-check-error-code.h"
#include "otter-trace/trace-static-constants.h"
#include "otter-trace/trace-common-event-attributes.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE EVENTS                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
trace_event_thread_begin(trace_location_def_t *self)
{
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    OTF2_DefWriter *def_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, &def_writer);

    trace_add_thread_attributes(self);
    OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_begin]
    );
    OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    OTF2_EvtWriter_ThreadBegin(
        evt_writer,
        attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        trace_location_get_id(self)
    );
    trace_location_inc_event_count(self);
    return;
}

void
trace_event_thread_end(trace_location_def_t *self)
{
    OTF2_AttributeList *attributes = NULL;
    OTF2_EvtWriter *evt_writer = NULL;
    OTF2_DefWriter *def_writer = NULL;
    trace_location_get_otf2(self, &attributes, &evt_writer, &def_writer);

    trace_add_thread_attributes(self);
    OTF2_AttributeList_AddStringRef(
        attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_end]
    );
    OTF2_AttributeList_AddStringRef(
        attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );
    OTF2_EvtWriter_ThreadEnd(
        evt_writer,    
        attributes, 
        get_timestamp(), 
        OTF2_UNDEFINED_COMM, 
        trace_location_get_id(self)
    );
    trace_location_inc_event_count(self);
    return;
}

void
trace_event_enter(
    trace_location_def_t *self,
    trace_region_def_t *region)
{
    LOG_ERROR_IF((region == NULL), "null region pointer");

    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, NULL, &evt_writer, NULL);

    if (trace_region_is_type(region, trace_region_parallel)) {
        trace_location_enter_region_def_scope(self);
    }

    if (trace_region_is_shared(region)) {
        trace_region_lock(region);
    }

    OTF2_AttributeList *attributes = trace_region_get_attribute_list(region);
    trace_region_attr_t attr = trace_region_get_attributes(region);
    trace_region_type_t region_type = trace_region_get_type(region);

    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        attributes,
        trace_region_get_encountering_task_id(region),
        region_type,
        attr
    );

    OTF2_StringRef event_type = OTF2_UNDEFINED_STRING;
    OTF2_StringRef endpoint = attr_label_ref[attr_endpoint_enter];

    switch (region_type) {
    case trace_region_parallel:
        event_type = attr_label_ref[attr_event_type_parallel_begin];  break;
    case trace_region_workshare:
        event_type = attr_label_ref[attr_event_type_workshare_begin]; break;
    case trace_region_synchronise:
        event_type = attr_label_ref[attr_event_type_sync_begin];      break;
    case trace_region_task:
        event_type = attr_label_ref[attr_event_type_task_enter];      break;
    case trace_region_master:
        event_type = attr_label_ref[attr_event_type_master_begin];    break;
    case trace_region_phase:
        event_type = attr_label_ref[attr_event_type_phase_begin];     break;
    }

    OTF2_AttributeList_AddStringRef(attributes, attr_event_type, event_type);
    OTF2_AttributeList_AddStringRef(attributes, attr_endpoint, endpoint);

    trace_add_region_type_attributes(region);

    /* Record the event */
    OTF2_EvtWriter_Enter(evt_writer, attributes, get_timestamp(), trace_region_get_ref(region));

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
    trace_region_def_t *region = NULL;
    trace_location_leave_region(self, &region);

    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, NULL, &evt_writer, NULL);

    if (trace_region_is_shared(region)) {
        trace_region_lock(region);
    }

    trace_region_type_t region_type = trace_region_get_type(region);

    OTF2_AttributeList *attributes = trace_region_get_attribute_list(region);

    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        attributes,
        trace_region_get_encountering_task_id(region),
        region_type,
        trace_region_get_attributes(region)
    );

    OTF2_StringRef event_type = OTF2_UNDEFINED_STRING;
    OTF2_StringRef endpoint = attr_label_ref[attr_endpoint_leave];

    switch (region_type) {
    case trace_region_parallel:
        event_type = attr_label_ref[attr_event_type_parallel_end];  break;
    case trace_region_workshare:
        event_type = attr_label_ref[attr_event_type_workshare_end]; break;
    case trace_region_synchronise:
        event_type = attr_label_ref[attr_event_type_sync_end];      break;
    case trace_region_task:
        event_type = attr_label_ref[attr_event_type_task_leave];    break;
    case trace_region_master:
        event_type = attr_label_ref[attr_event_type_master_end];    break;
    case trace_region_phase:
        event_type = attr_label_ref[attr_event_type_phase_end];     break;
    }

    OTF2_AttributeList_AddStringRef(attributes, attr_event_type, event_type);
    OTF2_AttributeList_AddStringRef(attributes, attr_endpoint, endpoint);

    trace_add_region_type_attributes(region);

    /* Record the event */
    OTF2_EvtWriter_Leave(evt_writer, attributes, get_timestamp(), trace_region_get_ref(region));

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
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, NULL, &evt_writer, NULL);

    trace_region_attr_t attr = trace_region_get_attributes(region);
    OTF2_StringRef event_type = attr_label_ref[attr_event_type_task_create];
    OTF2_StringRef endpoint = attr_label_ref[attr_endpoint_discrete];
    // https://releases.llvm.org/15.0.0/tools/clang/docs/ReleaseNotes.html#improvements-to-clang-s-diagnostics
    // The -Wint-conversion warning diagnostic for implicit int <-> pointer conversions now defaults to an error in all C language modes.
    uint64_t task_create_ra = (uint64_t) attr.task.task_create_ra;

    OTF2_AttributeList *attributes = trace_region_get_attribute_list(region);
    trace_add_common_event_attributes(
        attributes,
        trace_region_get_encountering_task_id(region),
        trace_region_get_type(region),
        attr
    );
    OTF2_AttributeList_AddStringRef(attributes, attr_event_type, event_type);
    OTF2_AttributeList_AddStringRef(attributes, attr_endpoint, endpoint);
    OTF2_AttributeList_AddUint64(attributes, attr_task_create_ra, task_create_ra);
    trace_add_task_attributes(region);
    
    OTF2_EvtWriter_ThreadTaskCreate(evt_writer, attributes, get_timestamp(), OTF2_UNDEFINED_COMM, OTF2_UNDEFINED_UINT32, 0);
    
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
    
    OTF2_EvtWriter *evt_writer = NULL;
    trace_location_get_otf2(self, NULL, &evt_writer, NULL);

    trace_region_set_task_status(prior_task, prior_status);

    trace_location_store_active_regions_in_task(self, prior_task);
    trace_location_get_active_regions_from_task(self, next_task);

    OTF2_AttributeList *attributes = trace_region_get_attribute_list(prior_task);
    trace_region_attr_t prior_task_attr = trace_region_get_attributes(prior_task);
    trace_region_attr_t next_task_attr = trace_region_get_attributes(next_task);

    trace_add_common_event_attributes(
        attributes,
        trace_region_get_encountering_task_id(prior_task),
        trace_region_get_type(prior_task),
        prior_task_attr
    );

    // Record the reason the task-switch event ocurred
    OTF2_AttributeList_AddStringRef(attributes, attr_prior_task_status, TASK_STATUS_TO_STR_REF(prior_status));

    // The task that was suspended
    OTF2_AttributeList_AddUint64(attributes, attr_prior_task_id, prior_task_attr.task.id);

    // The task that was resumed
    OTF2_AttributeList_AddUint64(attributes, attr_unique_id, next_task_attr.task.id);

    // The task that was resumed
    OTF2_AttributeList_AddUint64(attributes, attr_next_task_id, next_task_attr.task.id);

    // The region_type of the task that was resumed
    OTF2_AttributeList_AddStringRef(attributes, attr_next_task_region_type, TASK_TYPE_TO_STR_REF(next_task_attr.task.type));

    // Task-switch is always considered a discrete event
    OTF2_AttributeList_AddStringRef(attributes, attr_endpoint, attr_label_ref[attr_endpoint_discrete]);

    OTF2_AttributeList_AddStringRef(attributes, attr_event_type, attr_label_ref[attr_event_type_task_switch]);

    OTF2_EvtWriter_ThreadTaskSwitch(
        evt_writer,
        attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    return;
}
