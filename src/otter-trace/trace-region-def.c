#include <stdlib.h>
#include <pthread.h>
#include <otf2/otf2.h>
#include <assert.h>
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace-ompt.h"
#include "public/otter-trace/trace-region-def.h"

#include "otter-trace/trace-static-constants.h"
#include "otter-trace/trace-archive.h"
#include "otter-trace/trace-attributes.h"
#include "otter-trace/trace-check-error-code.h"
#include "otter-trace/trace-lookup-macros.h"
#include "otter-trace/trace-string-registry.h"
#include "otter-trace/trace-unique-refs.h"
#include "otter-trace/trace-common-event-attributes.h"

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
typedef struct trace_region_def_t {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    unique_id_t          encountering_task_id;
    otter_stack_t       *rgn_stack;
    trace_region_attr_t  attr;    
} trace_region_def_t;


// Constructors

trace_region_def_t *
trace_new_master_region(
    unique_id_t thread_id,
    unique_id_t encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_MASTER,
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_master,
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.master = {
            .thread = thread_id
        }
    };
    return new;    
}

trace_region_def_t *
trace_new_parallel_region(
    unique_id_t    id, 
    unique_id_t    master,
    unique_id_t    encountering_task_id,
    int            flags,
    unsigned int   requested_parallelism)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_PARALLEL,
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_parallel,
        .encountering_task_id = encountering_task_id,
        .rgn_stack = NULL,
        .attr.parallel = {
            .id            = id,
            .master_thread = master,
            .is_league     = flags & otter_parallel_league ? true : false,
            .requested_parallelism = requested_parallelism,
            .ref_count     = 0,
            .enter_count   = 0,
            .lock_rgn      = PTHREAD_MUTEX_INITIALIZER,
            .rgn_defs      = queue_create()
        }
    };
    return new;
}

trace_region_def_t *
trace_new_phase_region(
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
            .name = 0
        }
    };

    if (phase_name != NULL) {
        new->attr.phase.name = trace_register_string_with_lock(phase_name);
    } else {
        new->attr.phase.name = 0;
    }
    return new;
}

trace_region_def_t *
trace_new_sync_region(
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
            .sync_descendant_tasks = (task_sync_mode == trace_sync_descendants ? true : false)
        }
    };
    return new;
}

trace_region_def_t *
trace_new_task_region(
    trace_region_def_t    *parent_task_region, 
    unique_id_t            id,
    otter_task_flag_t      flags,
    int                    has_dependences,
    otter_src_location_t  *src_location,
    const void            *task_create_ra)
{
    /* Create a region representing a task. Add to the location's region
       definition queue. */

    /* A task maintains a stack of the active regions encountered during its
       execution up to a task-switch event, which is restored to the executing
       thread when the task is resumed */

    LOG_DEBUG_IF((src_location), "got src_location(file=%s, func=%s, line=%d)\n", src_location->file, src_location->func, src_location->line);

    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref = get_unique_rgn_ref(),
        .role = OTF2_REGION_ROLE_TASK,
        .attributes = OTF2_AttributeList_New(),
        .type = trace_region_task,
        .rgn_stack = stack_create(),
        .attr.task = {
            .id              = id,
            .type            = flags & otter_task_type_mask,
            .flags           = flags,
            .has_dependences = has_dependences,
            .parent_id   = parent_task_region != NULL ? 
                parent_task_region->attr.task.id   : OTF2_UNDEFINED_UINT64,
            .parent_type = parent_task_region != NULL ? 
                parent_task_region->attr.task.type : OTF2_UNDEFINED_UINT32,
            .task_status     = 0 /* no status */,
            .source_file_name_ref = 0,
            .source_func_name_ref = 0,
            .source_line_number = 0,
        }
    };
    new->encountering_task_id = new->attr.task.parent_id;

    if (src_location != NULL) {
        new->attr.task.source_file_name_ref = trace_register_string_with_lock(src_location->file);
        new->attr.task.source_func_name_ref = trace_register_string_with_lock(src_location->func);
        new->attr.task.source_line_number = src_location->line;
    } else {
        new->attr.task.source_file_name_ref = 0;
        new->attr.task.source_func_name_ref = 0;
        new->attr.task.source_line_number = 0;
    }
    return new;
}

trace_region_def_t *
trace_new_workshare_region(
    otter_work_t          wstype, 
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
    return new;
}


// Destructors

void 
trace_destroy_master_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_destroy_parallel_region(trace_region_def_t *rgn)
{
    if (rgn->type != trace_region_parallel)
    {
        LOG_ERROR("invalid region type %d", rgn->type);
        abort();
    }

    size_t n_defs = queue_length(rgn->attr.parallel.rgn_defs);
    LOG_DEBUG("[parallel=%lu] writing nested region definitions (%lu)", 
        rgn->attr.parallel.id, n_defs);

    pthread_mutex_t *lock_global_def_writer = global_def_writer_lock();

    /* Lock the global def writer first */
    pthread_mutex_lock(lock_global_def_writer);
    
    /* Write parallel region's definition */
    trace_write_region_definition(rgn);

    /* write region's nested region definitions */
    trace_region_def_t *r = NULL;
    int count=0;
    while (queue_pop(rgn->attr.parallel.rgn_defs, (data_item_t*) &r))
    {
        LOG_DEBUG("[parallel=%lu] writing region definition %d/%lu (region %3u)",
            rgn->attr.parallel.id, count+1, n_defs, r->ref);
        count++;
        trace_write_region_definition(r);

        /* destroy each region once its definition is written */
        switch (r->type)
        {
        case trace_region_workshare:
            trace_destroy_workshare_region(r);
            break;

        case trace_region_master:
            trace_destroy_master_region(r);
            break;
        
        case trace_region_synchronise:
            trace_destroy_sync_region(r);
            break;

        case trace_region_task:
            trace_destroy_task_region(r);
            break;

        case trace_region_phase:
            trace_destroy_phase_region(r);
            break;
        
        default:
            LOG_ERROR("unknown region type %d", r->type);
            abort();
        }
    }

    /* Release once done */
    pthread_mutex_unlock(lock_global_def_writer);

    /* destroy parallel region once all locations are done with it
       and all definitions written */
    OTF2_AttributeList_Delete(rgn->attributes);
    queue_destroy(rgn->attr.parallel.rgn_defs, false, NULL);
    LOG_DEBUG("region %p (parallel id %lu)", rgn, rgn->attr.parallel.id);
    free(rgn);
    return;
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
trace_destroy_sync_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void
trace_destroy_task_region(trace_region_def_t *rgn)
{
    LOG_WARN_IF(
        (!(rgn->attr.task.task_status == otter_task_complete 
            || rgn->attr.task.task_status == otter_task_cancel)),
        "destroying task region before task-complete/task-cancel");
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p destroying active regions stack %p", rgn, rgn->rgn_stack);
    stack_destroy(rgn->rgn_stack, false, NULL);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

void 
trace_destroy_workshare_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}


// Add attributes

void
trace_add_region_type_attributes(trace_region_def_t *rgn)
{
    switch (rgn->type) {
    case trace_region_parallel:
        trace_add_parallel_attributes(rgn);  break;
    case trace_region_workshare:
        trace_add_workshare_attributes(rgn); break;
    case trace_region_synchronise:
        trace_add_sync_attributes(rgn);      break;
    case trace_region_task:
        trace_add_task_attributes(rgn);      break;
    case trace_region_master:
        trace_add_master_attributes(rgn);    break;
    case trace_region_phase:
        trace_add_phase_attributes(rgn);     break;
    }
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

void
trace_add_parallel_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddUint64(rgn->attributes, attr_unique_id,
        rgn->attr.parallel.id);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint32(rgn->attributes, attr_requested_parallelism,
        rgn->attr.parallel.requested_parallelism);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_is_league,
        rgn->attr.parallel.is_league ? 
            attr_label_ref[attr_flag_true] : attr_label_ref[attr_flag_false]);
    CHECK_OTF2_ERROR_CODE(r);
    return;
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

void
trace_add_task_attributes(trace_region_def_t *rgn)
{
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_AttributeList_AddUint64(rgn->attributes, attr_unique_id,
        rgn->attr.task.id);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_task_type,
        TASK_TYPE_TO_STR_REF(rgn->attr.task.type));
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint32(rgn->attributes, attr_task_flags,
        rgn->attr.task.flags);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint64(rgn->attributes, attr_parent_task_id,
        rgn->attr.task.parent_id);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_parent_task_type,
        TASK_TYPE_TO_STR_REF(rgn->attr.task.parent_type));
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_has_dependences,
        rgn->attr.task.has_dependences);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_undeferred,
        rgn->attr.task.flags & otter_task_undeferred);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_untied,
        rgn->attr.task.flags & otter_task_untied);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_final,
        rgn->attr.task.flags & otter_task_final);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_mergeable,
        rgn->attr.task.flags & otter_task_mergeable);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_merged,
        rgn->attr.task.flags & otter_task_merged);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_prior_task_status,
        TASK_STATUS_TO_STR_REF(rgn->attr.task.task_status));
    CHECK_OTF2_ERROR_CODE(r);

    // Add source location if defined for this task
    if (rgn->attr.task.source_file_name_ref != 0)
    {
        r = OTF2_AttributeList_AddUint32(rgn->attributes, attr_source_line_number,
        rgn->attr.task.source_line_number);
        CHECK_OTF2_ERROR_CODE(r);
        r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_source_file_name,
            rgn->attr.task.source_file_name_ref);
        CHECK_OTF2_ERROR_CODE(r);
        r = OTF2_AttributeList_AddStringRef(rgn->attributes, attr_source_func_name,
            rgn->attr.task.source_func_name_ref);
        CHECK_OTF2_ERROR_CODE(r);
    }
    return;
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


// Getters

OTF2_AttributeList *
trace_region_get_attribute_list(trace_region_def_t *region)
{
    return region->attributes;
}

OTF2_RegionRef
trace_region_get_ref(trace_region_def_t *region)
{
    return region->ref;
}

unique_id_t 
trace_region_get_encountering_task_id(trace_region_def_t *region)
{
    return region->encountering_task_id;
}

trace_region_attr_t 
trace_region_get_attributes(trace_region_def_t *region)
{
    return region->attr;
}

trace_region_type_t
trace_region_get_type(trace_region_def_t *region)
{
    return region->type;
}

otter_queue_t *
trace_region_get_rgn_def_queue(trace_region_def_t *region)
{
    // This operation is only valid for parallel regions
    assert(region->type == trace_region_parallel);
    return region->attr.parallel.rgn_defs;
}

otter_stack_t *
trace_region_get_task_rgn_stack(trace_region_def_t *region)
{
    // This operation is only valid for task regions
    assert(region->type == trace_region_task);
    return region->rgn_stack;
}

unsigned int
trace_region_get_shared_ref_count(trace_region_def_t *region)
{
    assert(trace_region_is_shared(region));
    return region->attr.parallel.ref_count;
}


// Setters

void
trace_region_set_task_status(trace_region_def_t *region, otter_task_status_t status)
{
    assert(region->type == trace_region_task);
    region->attr.task.task_status = status;
}

// Lock and unlock shared regions

bool
trace_region_is_type(trace_region_def_t *region, trace_region_type_t region_type)
{
    return region->type == region_type;
}

bool
trace_region_is_shared(trace_region_def_t *region)
{
    return region->type == trace_region_parallel;
}

void
trace_region_lock(trace_region_def_t *region)
{
    assert(trace_region_is_shared(region));
    pthread_mutex_lock(&region->attr.parallel.lock_rgn);
}

void
trace_region_unlock(trace_region_def_t *region)
{
    assert(trace_region_is_shared(region));
    pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
}

void
trace_region_inc_ref_count(trace_region_def_t *region)
{
    assert(trace_region_is_shared(region));
    region->attr.parallel.ref_count++;
    region->attr.parallel.enter_count++;
}

void
trace_region_dec_ref_count(trace_region_def_t *region)
{
    assert(trace_region_is_shared(region));
    region->attr.parallel.ref_count--;
}


// Write region definition to a trace

void trace_region_write_definition_impl(OTF2_GlobalDefWriter *writer, trace_region_def_t *region)
{
    if (region == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }

    LOG_DEBUG("writing region definition %3u (type=%3d, role=%3u) %p",
        region->ref, region->type, region->role, region);

    switch (region->type)
    {
        case trace_region_parallel:
        {
            char region_name[DEFAULT_NAME_BUF_SZ+1] = {0};
            snprintf(region_name, DEFAULT_NAME_BUF_SZ, "Parallel Region %lu",
                region->attr.parallel.id);
            OTF2_StringRef region_name_ref = get_unique_str_ref();
            OTF2_GlobalDefWriter_WriteString(writer,
                region_name_ref,
                region_name);
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                region_name_ref,
                0, 0,   /* canonical name, description */
                region->role,
#if defined(OTTER_SERIAL_MODE)
                OTF2_PARADIGM_USER,
#else
                OTF2_PARADIGM_OPENMP,
#endif
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_workshare:
        {
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                WORK_TYPE_TO_STR_REF(region->attr.wshare.type),
                0, 0,
                region->role,
#if defined(OTTER_SERIAL_MODE)
                OTF2_PARADIGM_USER,
#else
                OTF2_PARADIGM_OPENMP,
#endif
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_master:
        {
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                attr_label_ref[attr_region_type_master],
                0, 0,
                region->role,
#if defined(OTTER_SERIAL_MODE)
                OTF2_PARADIGM_USER,
#else
                OTF2_PARADIGM_OPENMP,
#endif
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_synchronise:
        {
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                SYNC_TYPE_TO_STR_REF(region->attr.sync.type),
                0, 0,
                region->role,
#if defined(OTTER_SERIAL_MODE)
                OTF2_PARADIGM_USER,
#else
                OTF2_PARADIGM_OPENMP,
#endif
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_task:
        {
            char task_name[DEFAULT_NAME_BUF_SZ+1] = {0};
            snprintf(task_name, DEFAULT_NAME_BUF_SZ, "%s task %lu",
                region->attr.task.type == otter_task_initial ? "initial" :
                    region->attr.task.type == otter_task_implicit ? "implicit" :
                    region->attr.task.type == otter_task_explicit ? "explicit" :
                    region->attr.task.type == otter_task_target   ? "target" : "??",
                region->attr.task.id);
            OTF2_StringRef task_name_ref = get_unique_str_ref();
            OTF2_GlobalDefWriter_WriteString(writer, task_name_ref, task_name);
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                task_name_ref,
                0, 0,   /* canonical name, description */
                region->role,
                OTF2_PARADIGM_OPENMP,
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_phase:
        {
            OTF2_GlobalDefWriter_WriteRegion(writer,
                region->ref,
                PHASE_TYPE_TO_STR_REF(region->attr.sync.type),
                0, 0,
                region->role,
#if defined(OTTER_SERIAL_MODE)
                OTF2_PARADIGM_USER,
#else
                OTF2_PARADIGM_OPENMP,
#endif
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        default:
        {
            LOG_ERROR("unexpected region type %d", region->type);
        }
    }
    return;
}
