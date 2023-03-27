#define _GNU_SOURCE

#include <otf2/otf2.h>
#include <time.h>
#include <pthread.h>
#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace.h"
#include "src/otter-trace/trace-attributes.h"
#include "src/otter-trace/trace-lookup-macros.h"
#include "private/otter-trace/trace-unique-refs.h"
#include "src/otter-trace/trace-check-error-code.h"
#include "private/otter-task-graph/otter-task-context-interface.h"

#define OTTER_DUMMY_OTF2_LOCATION_REF        0

/* Protects the shared dummy event writer object */
static pthread_mutex_t lock_shared_evt_writer = PTHREAD_MUTEX_INITIALIZER;

static OTF2_EvtWriter *get_shared_event_writer(void) {
    LOG_DEBUG("locking shared event writer");
    pthread_mutex_lock(&lock_shared_evt_writer);
    return OTF2_Archive_GetEvtWriter(
        get_global_archive(),
        OTTER_DUMMY_OTF2_LOCATION_REF
    );
}

static void release_shared_event_writer(void) {
    LOG_DEBUG("releasing shared event writer");
    pthread_mutex_unlock(&lock_shared_evt_writer);
}

static uint64_t get_timestamp(void);

static void trace_add_common_event_attributes(
    OTF2_AttributeList *attributes,
    unique_id_t encountering_task_id,
    trace_region_type_t region_type,
    trace_region_attr_t region_attr);

/* Lookup tables mapping enum value to string ref */
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

void trace_graph_event_task_begin(otter_task_context *task, trace_region_attr_t task_attr)
{
    /*
    Record event: OTF2_EvtWriter_ThreadTaskSwitch()
        - Attach any relevant attributes to task->attributes i.e. endpoint
    NOTE: if parent == NULL i.e. the new task is an orphan, it won't be possible later to exclude its execution time from that of any parent task
    */

    OTF2_ErrorCode err;
    LOG_DEBUG("record task-graph event: task begin");

    // Add relevant attributes
    OTF2_AttributeList *attr = OTF2_AttributeList_New();

    unique_id_t task_id = otterTaskContext_get_task_context_id(task);
    unique_id_t parent_task_id = otterTaskContext_get_parent_task_context_id(task);
    
    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        attr,
        parent_task_id,
        trace_region_task,
        task_attr
    );
    
    // Event type
    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Task context ID
    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_unique_id,
        task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Parent task context ID
    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_parent_task_id,
        parent_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Endpoint: enter
    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Record event

    OTF2_EvtWriter *event_writer = NULL;

    event_writer = get_shared_event_writer();

    OTF2_EvtWriter_ThreadTaskSwitch(
        event_writer,
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    release_shared_event_writer();

    CHECK_OTF2_ERROR_CODE(err);

    OTF2_AttributeList_Delete(attr);
}

void trace_graph_event_task_end(otter_task_context *task)
{
    OTF2_ErrorCode err;
    LOG_DEBUG("record task-graph event: task end");
    unique_id_t task_id = otterTaskContext_get_task_context_id(task);

    // Add attributes to task
    OTF2_AttributeList *attr = OTF2_AttributeList_New();
    
    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        attr,
        task_id,
        trace_region_task,
        (trace_region_attr_t) {
            .task = {
                .type = otter_task_explicit,
                .id = task_id
            }
        }
    );
    
    // Event type
    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Task context ID
    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_unique_id,
        otterTaskContext_get_task_context_id(task)
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Parent task context ID
    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_parent_task_id,
        otterTaskContext_get_parent_task_context_id(task)
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Endpoiont: leave
    OTF2_AttributeList_AddStringRef(
        attr,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );

    // Record event

    OTF2_EvtWriter *event_writer = get_shared_event_writer();

    OTF2_EvtWriter_ThreadTaskSwitch(
        event_writer,
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    release_shared_event_writer();

    CHECK_OTF2_ERROR_CODE(err);
    
    OTF2_AttributeList_Delete(attr);
}

void trace_graph_synchronise_tasks(otter_task_context *task, trace_region_attr_t sync_attr)
{
    LOG_DEBUG("record task-graph event: synchronise");

    OTF2_AttributeList *attributes = OTF2_AttributeList_New();
    
    /* Add attributes common to all enter/leave events */
    trace_add_common_event_attributes(
        attributes,
        otterTaskContext_get_task_context_id(task),
        trace_region_synchronise,
        sync_attr
    );

    /* Add the event type attribute */
    OTF2_AttributeList_AddStringRef(attributes, attr_event_type,
        attr_label_ref[attr_event_type_sync_begin]
    );

    /* Add the endpoint */
    OTF2_AttributeList_AddStringRef(attributes, attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]);

    /* Add the sync mode - children or descendants? */
    OTF2_AttributeList_AddUint8(attributes, attr_sync_descendant_tasks, sync_attr.sync.sync_descendant_tasks ? 1 : 0);

    /* Get the event writer */
    OTF2_EvtWriter *event_writer = get_shared_event_writer();

    OTF2_EvtWriter_Enter(
        event_writer,
        attributes,
        get_timestamp(),
        OTF2_UNDEFINED_REGION
    );

    release_shared_event_writer();
    
    OTF2_AttributeList_Delete(attributes);
}

static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}

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
        region_type == 
#if defined(USE_OMPT_MASKED)
            trace_region_masked 
#else
            trace_region_master
#endif
        ? attr_label_ref[attr_region_type_master]   :
        region_type == trace_region_phase ?
            PHASE_TYPE_TO_STR_REF(region_attr.phase.type) :
        attr_label_ref[attr_region_type_task]
    );
    CHECK_OTF2_ERROR_CODE(r);

    return;
}
