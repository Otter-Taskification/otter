#include <otf2/otf2.h>
#include "public/otter-common.h"
#include "public/otter-trace/trace-region-attr.h"

#include "otter-trace/trace-attributes.h"
#include "otter-trace/trace-region-types.h"
#include "otter-trace/trace-check-error-code.h"

/* Lookup tables mapping enum value to string ref */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

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
