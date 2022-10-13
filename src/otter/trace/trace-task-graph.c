#define _GNU_SOURCE

#include <otf2/otf2.h>
#include "otter/debug.h"
#include "otter/otter-common.h"
#include "otter/otter-environment-variables.h"
#include "otter/trace.h"
#include "otter/trace-attributes.h"
#include "otter/trace-lookup-macros.h"
#include "otter/trace-unique-refs.h"
#include "otter/trace-check-error-code.h"
#include "otter/otter-task-context-interface.h"

#define OTTER_DUMMY_OTF2_LOCATION_REF        0

void trace_graph_event_task_begin(otter_task_context *task)
{
    /*
    Record event: OTF2_EvtWriter_ThreadTaskSwitch()
        - Attach any relevant attributes to task->attributes i.e. endpoint
    NOTE: if parent == NULL i.e. the new task is an orphan, it won't be possible later to exclude its execution time from that of any parent task
    */

    OTF2_ErrorCode err;
    LOG_DEBUG("record task-graph event: task begin");

    // Add relevant attributes
    OTF2_AttributeList *attr = otterTaskContext_get_attribute_list(task);

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

    // Endpoint: enter
    err = OTF2_AttributeList_AddStringRef(
        attr,
        attr_endpoint,
        attr_endpoint_enter
    );
    CHECK_OTF2_ERROR_CODE(err);

    // Record event

    OTF2_EvtWriter *event_writer = OTF2_Archive_GetEvtWriter(
        get_global_archive(),
        OTTER_DUMMY_OTF2_LOCATION_REF
    );

    OTF2_EvtWriter_ThreadTaskSwitch(
        event_writer,
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    CHECK_OTF2_ERROR_CODE(err);
}

void trace_graph_event_task_end(otter_task_context *task)
{
    OTF2_ErrorCode err;
    LOG_DEBUG("record task-graph event: task end");

    // Add attributes to task
    OTF2_AttributeList *attr = otterTaskContext_get_attribute_list(task);

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
        attr_endpoint_leave
    );

    // Record event

    OTF2_EvtWriter *event_writer = OTF2_Archive_GetEvtWriter(
        get_global_archive(),
        OTTER_DUMMY_OTF2_LOCATION_REF
    );

    OTF2_EvtWriter_ThreadTaskSwitch(
        event_writer,
        attr,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */

    CHECK_OTF2_ERROR_CODE(err);
}
