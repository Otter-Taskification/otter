#include <stdlib.h>
#include <pthread.h>
#include "otter/trace-lookup-macros.h"
#include "otter/trace-enum-types.h"
#include "otter/trace-attributes.h"
#include "otter/trace-structs.h"
#include "otter/trace-region-sync.h"
#include "otter/trace-unique-refs.h"
#include "otter/trace-check-error-code.h"
#include "otter/queue.h"
#include "otter/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

trace_region_def_t *
trace_new_sync_region(
    trace_location_def_t *loc,
    otter_sync_region_t   stype,
    trace_task_sync_t     task_sync_mode,
    unique_id_t           encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = SYNC_TYPE_TO_OTF2_REGION_ROLE(stype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_synchronise,
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.sync = {
            .type = stype,
            .sync_descendant_tasks = (task_sync_mode == sync_descendants ? true : false)
        }
    };

    LOG_DEBUG("[t=%lu] created sync region %u at %p",
        loc->id, new->ref, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;
}

void
trace_destroy_sync_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_add_sync_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_sync_type,
        SYNC_TYPE_TO_STR_REF(rgn->attr.sync.type));
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_sync_descendant_tasks,
        (uint8_t)(rgn->attr.sync.sync_descendant_tasks ? 1 : 0)
    );
    return;
}
