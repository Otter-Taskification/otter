#define _GNU_SOURCE

#include <otf2/otf2.h>
#include <time.h>
#include <pthread.h>
#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-task-graph.h"
#include "public/otter-trace/trace-task-context-interface.h"

#include "otter-trace/trace-archive-impl.h"
#include "otter-trace/trace-types-as-labels.h"
#include "otter-trace/trace-timestamp.h"
#include "otter-trace/trace-attributes.h"
#include "otter-trace/trace-unique-refs.h"
#include "otter-trace/trace-check-error-code.h"
#include "otter-trace/trace-attribute-lookup.h"

#define OTTER_DUMMY_OTF2_LOCATION_REF        0

/* Protects the shared dummy event writer object */
// TODO: refactor global state
static pthread_mutex_t lock_shared_evt_writer = PTHREAD_MUTEX_INITIALIZER;

static inline OTF2_EvtWriter *get_shared_event_writer(void) {
    LOG_DEBUG("locking shared event writer");
    pthread_mutex_lock(&lock_shared_evt_writer);
    return OTF2_Archive_GetEvtWriter(
        // TODO: replace global state with injected state
        get_global_archive(),
        OTTER_DUMMY_OTF2_LOCATION_REF
    );
}

static inline void release_shared_event_writer(void) {
    // TODO: replace global state with injected state
    LOG_DEBUG("releasing shared event writer");
    pthread_mutex_unlock(&lock_shared_evt_writer);
}

// TODO: accept injected state
void trace_graph_event_task_begin(otter_task_context *task, trace_task_region_attr_t task_attr)
{
    /*
    Record event: OTF2_EvtWriter_ThreadTaskSwitch()
        - Attach any relevant attributes to task->attributes i.e. endpoint
    NOTE: if parent == NULL i.e. the new task is an orphan, it won't be possible later to exclude its execution time from that of any parent task
    */

    LOG_DEBUG("record task-graph event: task begin");

    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attr = OTF2_AttributeList_New();
    unique_id_t task_id = otterTaskContext_get_task_context_id(task);
    unique_id_t parent_task_id = otterTaskContext_get_parent_task_context_id(task);

    err = OTF2_AttributeList_AddInt32(
        attr,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_encountering_task_id,
        parent_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_region_type,
        attr_label_ref[task_type_as_label(task_attr.type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_unique_id,
        task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_parent_task_id,
        parent_task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_endpoint,
        attr_label_ref[attr_endpoint_enter]
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Record event
    err = OTF2_EvtWriter_ThreadTaskSwitch(
        // TODO: replace global state with injected state
        get_shared_event_writer(),
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
    CHECK_OTF2_ERROR_CODE(err);

    // Cleanup
    // TODO: replace global state with injected state
    release_shared_event_writer();
    OTF2_AttributeList_Delete(attr);
}

// TODO: accept injected state
void trace_graph_event_task_end(otter_task_context *task)
{
    LOG_DEBUG("record task-graph event: task end");

    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attr = OTF2_AttributeList_New();
    unique_id_t task_id = otterTaskContext_get_task_context_id(task);
    
    err = OTF2_AttributeList_AddInt32(
        attr,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_encountering_task_id,
        task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_region_type,
        attr_label_ref[attr_task_type_explicit_task]
    );
    CHECK_OTF2_ERROR_CODE(err);
    
    
    // Event type

    // Event type
    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_event_type,
        attr_label_ref[attr_event_type_task_switch]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_unique_id,
        task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_parent_task_id,
        otterTaskContext_get_parent_task_context_id(task)
    );
    CHECK_OTF2_ERROR_CODE(err);

    OTF2_AttributeList_AddStringRef(
        attr,
        attr_endpoint,
        attr_label_ref[attr_endpoint_leave]
    );

    err = OTF2_EvtWriter_ThreadTaskSwitch(
        // TODO: replace global state with injected state
        get_shared_event_writer(),
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
    CHECK_OTF2_ERROR_CODE(err);

    // Cleanup
    // TODO: replace global state with injected state
    release_shared_event_writer();
    OTF2_AttributeList_Delete(attr);
}

// TODO: accept injected state
void trace_graph_synchronise_tasks(otter_task_context *task, trace_sync_region_attr_t sync_attr)
{
    LOG_DEBUG("record task-graph event: synchronise");

    OTF2_ErrorCode err = OTF2_SUCCESS;
    OTF2_AttributeList *attr = OTF2_AttributeList_New();
    unique_id_t task_id = otterTaskContext_get_task_context_id(task);
    
    err = OTF2_AttributeList_AddInt32(
        attr,
        attr_cpu,
        sched_getcpu()
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint64(
        attr,
        attr_encountering_task_id,
        task_id
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_region_type,
        attr_label_ref[sync_type_as_label(sync_attr.type)]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_event_type,
        attr_label_ref[attr_event_type_sync_begin]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddStringRef(
        attr, 
        attr_endpoint,
        attr_label_ref[attr_endpoint_discrete]
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_AttributeList_AddUint8(
        attr, 
        attr_sync_descendant_tasks, 
        sync_attr.sync_descendant_tasks ? 1 : 0
    );
    CHECK_OTF2_ERROR_CODE(err);

    err = OTF2_EvtWriter_Enter(
        // TODO: replace global state with injected state
        get_shared_event_writer(),
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_REGION
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Cleanup
    // TODO: replace global state with injected state
    release_shared_event_writer();
    OTF2_AttributeList_Delete(attr);
}
