#include <stdlib.h>
#include <pthread.h>
#include "src/otter-trace/trace-lookup-macros.h"
#include "src/otter-trace/trace-attributes.h"
#include "public/otter-trace/trace-region-master.h"
#include "private/otter-trace/trace-unique-refs.h"
#include "src/otter-trace/trace-check-error-code.h"
#include "public/types/queue.h"
#include "public/types/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

trace_region_def_t *
trace_new_master_region(
    trace_location_def_t *loc,
    unique_id_t           encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_MASTER,
        .attributes = OTF2_AttributeList_New(),
#if defined(USE_OMPT_MASKED)
        .type       = trace_region_masked,
#else
        .type       = trace_region_master,
#endif
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.master = {
            .thread = loc->id
        }
    };

    LOG_DEBUG("[t=%lu] created master region %u at %p",
        loc->id, new->ref, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;    
}

void 
trace_destroy_master_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_add_master_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddUint64(rgn->attributes, attr_unique_id,
        rgn->attr.master.thread);
    CHECK_OTF2_ERROR_CODE(r);
    return;
}