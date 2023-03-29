// TODO: this file contains events for the OMPT event model as well as functions
// TODO: for adding region attributes to an attribute list and writing a region
// TODO: definition to a trace. Probably want to decouple these.

// TODO: fix pointer to incomplete type (use getters instead)

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <otf2/otf2.h>
#include <otf2/OTF2_Pthread_Locks.h>

#include "public/debug.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace.h"
#include "public/otter-trace/trace-location.h"

#include "src/otter-trace/trace-attributes.h"
#include "src/otter-trace/trace-archive.h"
#include "src/otter-trace/trace-lookup-macros.h"
#include "src/otter-trace/trace-unique-refs.h"
#include "src/otter-trace/trace-check-error-code.h"
#include "src/otter-trace/trace-static-constants.h"

static uint64_t get_timestamp(void);

/* apply a region's attributes to an event */
static void trace_add_common_event_attributes(
    OTF2_AttributeList *attributes,
    unique_id_t encountering_task_id,
    trace_region_type_t region_type,
    trace_region_attr_t region_attr);

/* Lookup tables mapping enum value to string ref */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

/* Mutexes for thread-safe access to Archive and Defs */
// extern pthread_mutex_t lock_global_def_writer;
// extern pthread_mutex_t lock_global_archive;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE DEFINITIONS                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO: this is used by otter-ompt and otter-serial, so want to allow passing in state here
void
trace_write_region_definition(trace_region_def_t *rgn)
{
    trace_region_write_definition_impl(get_global_def_writer(), rgn);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   ADD LOCATION/REGION ATTRIBUTES BEFORE RECORDING EVENTS                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
trace_add_common_event_attributes(
    OTF2_AttributeList *attributes,
    unique_id_t encountering_task_id,
    trace_region_type_t region_type,
    trace_region_attr_t region_attr)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;

    /* CPU of encountering thread */
    r = OTF2_AttributeList_AddInt32(attributes, attr_cpu, sched_getcpu());
    CHECK_OTF2_ERROR_CODE(r);

    /* Add encountering task ID */
    r = OTF2_AttributeList_AddUint64(
        attributes,
        attr_encountering_task_id,
        encountering_task_id
    );
    CHECK_OTF2_ERROR_CODE(r);

    /* Add the region type */
    r = OTF2_AttributeList_AddStringRef(attributes, attr_region_type,
        region_type == trace_region_parallel ?
            attr_label_ref[attr_region_type_parallel] :
        region_type == trace_region_workshare ?
            WORK_TYPE_TO_STR_REF(region_attr.wshare.type) :
        region_type == trace_region_synchronise ?
            SYNC_TYPE_TO_STR_REF(region_attr.sync.type) :
        region_type == trace_region_task ? 
            TASK_TYPE_TO_STR_REF(region_attr.task.type) :
        region_type == trace_region_master ?
            attr_label_ref[attr_region_type_master]   :
        region_type == trace_region_phase ?
            PHASE_TYPE_TO_STR_REF(region_attr.phase.type) :
        attr_label_ref[attr_region_type_task]
    );
    CHECK_OTF2_ERROR_CODE(r);

    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE EVENTS                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// TODO: lots of access of trace_location_def_t and trace_region_def_t internals to factor out

void
trace_event_thread_begin(trace_location_def_t *self)
{
    trace_add_thread_attributes(self);
    OTF2_AttributeList_AddStringRef(
        self->attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_begin]
    );
    OTF2_AttributeList_AddStringRef(
        self->attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    OTF2_EvtWriter_ThreadBegin(
        self->evt_writer,
        self->attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        self->id
    );
    self->events++;
    return;
}

void
trace_event_thread_end(trace_location_def_t *self)
{
    trace_add_thread_attributes(self);
    OTF2_AttributeList_AddStringRef(
        self->attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_thread_end]
    );
    OTF2_AttributeList_AddStringRef(
        self->attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );
    OTF2_EvtWriter_ThreadEnd(
        self->evt_writer,    
        self->attributes, 
        get_timestamp(), 
        OTF2_UNDEFINED_COMM, 
        self->id
    );
    self->events++;
    return;
}

void
trace_event_enter(
    trace_location_def_t *self,
    trace_region_def_t *region)
{
    LOG_ERROR_IF((region == NULL), "null region pointer");

    LOG_DEBUG("[t=%lu] enter region %p", self->id, region);

#if !defined(NDEBUG) && DEBUG_LEVEL>=3
    trace_region_pprint(stderr, region, __func__, __LINE__);
#endif

    #if DEBUG_LEVEL >= 4
    stack_print(self->rgn_stack);
    #endif

    if (region->type == trace_region_parallel)
    {
        /* Set up new region definitions queue for the new parallel region */
        stack_push(self->rgn_defs_stack, (data_item_t) {.ptr = self->rgn_defs});
        LOG_DEBUG("[t=%lu] pushed region definitions queue %p",
            self->id, self->rgn_defs);
        self->rgn_defs = queue_create();

        /* Parallel regions must be accessed atomically as they are shared 
           between threads */
        LOG_DEBUG("[t=%lu] acquiring mutex %p",
            self->id, &region->attr.parallel.lock_rgn);
        pthread_mutex_lock(&region->attr.parallel.lock_rgn);
        LOG_DEBUG("[t=%lu] acquired mutex %p",
            self->id, &region->attr.parallel.lock_rgn);
    }

    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        region->attributes,
        region->encountering_task_id,
        region->type,
        region->attr
    );

    /* Add the event type attribute */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_event_type,
        region->type == trace_region_parallel ?
            attr_label_ref[attr_event_type_parallel_begin] :
        region->type == trace_region_workshare ?
            attr_label_ref[attr_event_type_workshare_begin] :
        region->type == trace_region_synchronise ?
            attr_label_ref[attr_event_type_sync_begin] :
        region->type == trace_region_master ?
            attr_label_ref[attr_event_type_master_begin] :
        region->type == trace_region_phase ?
            attr_label_ref[attr_event_type_phase_begin] :
        attr_label_ref[attr_event_type_task_enter]
    );

    /* Add the endpoint */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_endpoint,
        attr_label_ref[attr_endpoint_enter]);

    /* Add region's attributes to the event */
    switch (region->type) {
    case trace_region_parallel: trace_add_parallel_attributes(region); break;
    case trace_region_workshare: trace_add_workshare_attributes(region); break;
    case trace_region_synchronise: trace_add_sync_attributes(region); break;
    case trace_region_task: trace_add_task_attributes(region); break;
    case trace_region_master: trace_add_master_attributes(region); break;
    case trace_region_phase: trace_add_phase_attributes(region); break;
    default:
        LOG_ERROR("unhandled region type %d", region->type);
        abort();
    }
    
    /* Record the event */
    OTF2_EvtWriter_Enter(self->evt_writer, 
        region->attributes, get_timestamp(), region->ref);

    /* Push region onto location's region stack */
    stack_push(self->rgn_stack, (data_item_t) {.ptr = region});

    if (region->type == trace_region_parallel)
    {
        region->attr.parallel.ref_count++;
        region->attr.parallel.enter_count++;
        LOG_INFO("[t=%lu] releasing mutex %p (ref count of parallel region %lu"
            " is %u)",
            self->id, &region->attr.parallel.lock_rgn,
            region->attr.parallel.id, region->attr.parallel.ref_count);
        pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
    }

    self->events++;
    return;
}

void
trace_event_leave(trace_location_def_t *self)
{
    #if DEBUG_LEVEL >= 4
    stack_print(self->rgn_stack);
    #endif

    if (stack_is_empty(self->rgn_stack))
    {
        LOG_ERROR("stack is empty");
        abort();
    }

    /* For the region-end event, the region was previously pushed onto the 
       location's region stack so should now be at the top (as long as regions
       are correctly nested) */
    trace_region_def_t *region = NULL;
    stack_pop(self->rgn_stack, (data_item_t*) &region);

    LOG_DEBUG("[t=%lu] leave region %p", self->id, region);

#if !defined(NDEBUG) && DEBUG_LEVEL>=3
    trace_region_pprint(stderr, region, __func__, __LINE__);
#endif

    if (region->type == trace_region_parallel)
    {
        /* Parallel regions must be accessed atomically as they are shared 
           between threads */
        LOG_DEBUG("[t=%lu] acquiring mutex %p",
            self->id, &region->attr.parallel.lock_rgn);
        pthread_mutex_lock(&region->attr.parallel.lock_rgn);
        LOG_DEBUG("[t=%lu] acquired mutex %p",
            self->id, &region->attr.parallel.lock_rgn);
    }

    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        region->attributes,
        region->encountering_task_id,
        region->type,
        region->attr
    );

    /* Add the event type attribute */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_event_type,
        region->type == trace_region_parallel ?
            attr_label_ref[attr_event_type_parallel_end] :
        region->type == trace_region_workshare ?
            attr_label_ref[attr_event_type_workshare_end] :
        region->type == trace_region_synchronise ?
            attr_label_ref[attr_event_type_sync_end] :
        region->type == trace_region_master ? 
            attr_label_ref[attr_event_type_master_end] :
        region->type == trace_region_phase ?
            attr_label_ref[attr_event_type_phase_end] :
        attr_label_ref[attr_event_type_task_leave]
    );

    /* Add the endpoint */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_endpoint,
        attr_label_ref[attr_endpoint_leave]);

    /* Add region's attributes to the event */
    switch (region->type) {
    case trace_region_parallel: trace_add_parallel_attributes(region); break;
    case trace_region_workshare: trace_add_workshare_attributes(region); break;
    case trace_region_synchronise: trace_add_sync_attributes(region); break;
    case trace_region_task: trace_add_task_attributes(region); break;
    case trace_region_master: trace_add_master_attributes(region); break;
    case trace_region_phase: trace_add_phase_attributes(region); break;
    default:
        LOG_ERROR("unhandled region type %d", region->type);
        abort();
    }

    /* Record the event */
    OTF2_EvtWriter_Leave(self->evt_writer, region->attributes, get_timestamp(),
        region->ref);
    
    /* Parallel regions must be cleaned up by the last thread to leave */
    if (region->type == trace_region_parallel)
    {
        /* Give the location's region definitions to the parallel region */
        LOG_DEBUG("[t=%lu] appending %lu region definitions to "
            "parallel region queue %p", self->id, queue_length(self->rgn_defs),
            region->attr.parallel.rgn_defs);

        if (!queue_append(region->attr.parallel.rgn_defs, self->rgn_defs))
            LOG_ERROR("error appending items to queue");

        /* Destroy region definitions queue */
        LOG_DEBUG("[t=%lu] destroying region definitions queue %p",
            self->id, self->rgn_defs);
        queue_destroy(self->rgn_defs, false, NULL);
        self->rgn_defs = NULL;

        /* Pop queue of enclosing parallel region (if there is one) */
        stack_pop(self->rgn_defs_stack, (data_item_t*) &self->rgn_defs);
        LOG_DEBUG("[t=%lu] popped region definitions queue %p",
            self->id, self->rgn_defs);

        region->attr.parallel.ref_count--;

        LOG_INFO("[t=%lu] releasing mutex %p (ref count of parallel region %lu"
            " is %u)",
            self->id, &region->attr.parallel.lock_rgn,
            region->attr.parallel.id, region->attr.parallel.ref_count);

        /* Check the ref count atomically __before__ unlocking */
        if (region->attr.parallel.ref_count == 0)
        {
            pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
            trace_destroy_parallel_region(region);
        } else {
            pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
        }
    }
    
    self->events++;
    return;
}

void
trace_event_task_create(
    trace_location_def_t *self, 
    trace_region_def_t   *created_task)
{
    trace_add_common_event_attributes(
        created_task->attributes,
        created_task->encountering_task_id,
        created_task->type,
        created_task->attr
    );

    /* task-create */
    OTF2_AttributeList_AddStringRef(created_task->attributes, attr_event_type,
        attr_label_ref[attr_event_type_task_create]);

    /* discrete event (no duration) */
    OTF2_AttributeList_AddStringRef(
        created_task->attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]
    );

    /* return address */
    OTF2_AttributeList_AddUint64(created_task->attributes, attr_task_create_ra,
// https://releases.llvm.org/15.0.0/tools/clang/docs/ReleaseNotes.html#improvements-to-clang-s-diagnostics
// The -Wint-conversion warning diagnostic for implicit int <-> pointer conversions now defaults to an error in all C language modes.
        (uint64_t) created_task->attr.task.task_create_ra
    );

    trace_add_task_attributes(created_task);
    
    OTF2_EvtWriter_ThreadTaskCreate(
        self->evt_writer,
        created_task->attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
    self->events++;
    return;
}

void 
trace_event_task_schedule(
    trace_location_def_t    *self,
    trace_region_def_t      *prior_task,
    otter_task_status_t      prior_status)
{
    /* Update prior task's status before recording task enter/leave events */
    LOG_ERROR_IF((prior_task->type != trace_region_task),
        "invalid region type %d", prior_task->type);
    prior_task->attr.task.task_status = prior_status;
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

    prior_task->attr.task.task_status = prior_status;
    LOG_ERROR_IF((stack_is_empty(prior_task->rgn_stack) == false),
        "prior task %lu region stack not empty",
        prior_task->attr.task.id);
    stack_transfer(prior_task->rgn_stack, self->rgn_stack);
    stack_transfer(self->rgn_stack, next_task->rgn_stack);

    trace_add_common_event_attributes(
        prior_task->attributes,
        prior_task->attr.task.id,
        prior_task->type,
        prior_task->attr
    );

    // Record the reason the task-switch event ocurred
    OTF2_AttributeList_AddStringRef(
        prior_task->attributes,
        attr_prior_task_status,
        TASK_STATUS_TO_STR_REF(prior_status)
    );

    // The task that was suspended
    OTF2_AttributeList_AddUint64(
        prior_task->attributes,
        attr_prior_task_id,
        prior_task->attr.task.id
    );

    // The task that was resumed
    OTF2_AttributeList_AddUint64(
        prior_task->attributes,
        attr_unique_id,
        next_task->attr.task.id
    );

    // The task that was resumed
    OTF2_AttributeList_AddUint64(
        prior_task->attributes,
        attr_next_task_id,
        next_task->attr.task.id
    );

    // The region_type of the task that was resumed
    OTF2_AttributeList_AddStringRef(
        prior_task->attributes,
        attr_next_task_region_type,
        TASK_TYPE_TO_STR_REF(next_task->attr.task.type)
    );

    // Task-switch is always considered a discrete event
    OTF2_AttributeList_AddStringRef(
        prior_task->attributes,
        attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]
    );

    OTF2_AttributeList_AddStringRef(
        prior_task->attributes,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );

    OTF2_EvtWriter_ThreadTaskSwitch(
        self->evt_writer,
        prior_task->attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  TIMESTAMP & UNIQUE REFERENCES                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}
