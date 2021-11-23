#include <stdlib.h>
#include <pthread.h>
#include "otter/trace-lookup-macros.h"
#include "otter/trace-attributes.h"
#include "otter/trace-structs.h"
#include "otter/trace-region-workshare.h"
#include "otter/queue.h"
#include "otter/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

trace_region_def_t *
trace_new_workshare_region(
    trace_location_def_t *loc,
    ompt_work_t           wstype, 
    uint64_t              count,
    unique_id_t           encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = WORK_TYPE_TO_OTF2_REGION_ROLE(wstype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_workshare,
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.wshare = {
            .type       = wstype,
            .count      = count
        }
    };

    LOG_DEBUG("[t=%lu] created workshare region %u at %p",
        loc->id, new->ref, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;
}

void 
trace_destroy_workshare_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_add_workshare_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_workshare_type,
        WORK_TYPE_TO_STR_REF(rgn->attr.wshare.type));
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint64(rgn->attributes, attr_workshare_count,
        rgn->attr.wshare.count);
    CHECK_OTF2_ERROR_CODE(r);
    return;
}