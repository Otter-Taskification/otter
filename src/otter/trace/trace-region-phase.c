#include <stdlib.h>
#include <pthread.h>
#include "otter/trace-lookup-macros.h"
#include "otter/trace-enum-types.h"
#include "otter/trace-attributes.h"
#include "otter/trace-structs.h"
#include "otter/trace-region-phase.h"
#include "otter/trace-unique-refs.h"
#include "otter/trace-check-error-code.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/string_value_registry.hpp"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

trace_region_def_t *
trace_new_phase_region(
    trace_location_def_t *loc,
    otter_phase_region_t  type,
    unique_id_t           encountering_task_id,
    const char           *phase_name)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_CODE,
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_phase,
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.phase = {
            .type = type,
            .name = phase_name ? string_registry_insert(get_global_str_registry(), phase_name) : 0
        }
    };

    LOG_DEBUG("[t=%lu] created region for phase \"%s\" (%u) at %p",
        loc->id, phase_name, new->ref, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;
}

void
trace_destroy_phase_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_add_phase_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_phase_type,
        PHASE_TYPE_TO_STR_REF(rgn->attr.phase.type));
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_phase_name,
        rgn->attr.phase.name);
    CHECK_OTF2_ERROR_CODE(r);
    return;
}
