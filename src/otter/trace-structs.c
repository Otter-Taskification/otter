#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <otf2/otf2.h>
#include <otf2/OTF2_Pthread_Locks.h>

#include "otter/general.h"
#include "otter/debug.h"
#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"
#include "otter/trace.h"
#include "otter/trace-lookup-macros.h"
#include "otter/trace-structs.h"

#include "otter/queue.h"
#include "otter/stack.h"

/* Defined in trace.c */
extern OTF2_Archive *Archive;
extern OTF2_GlobalDefWriter *Defs;
extern pthread_mutex_t lock_global_def_writer;
extern pthread_mutex_t lock_global_archive;

/* * * * * * * * * * * * * * * * */
/* * * * * Constructors  * * * * */
/* * * * * * * * * * * * * * * * */

trace_location_def_t *
trace_new_location_definition(
    unique_id_t            id,
    ompt_thread_t          thread_type,
    OTF2_LocationType      loc_type,
    OTF2_LocationGroupRef  loc_grp)
{
    trace_location_def_t *new = malloc(sizeof(*new));

    *new = (trace_location_def_t) {
        .id             = id,
        .thread_type    = thread_type,
        .events         = 0,
        .ref            = get_unique_loc_ref(),
        .type           = loc_type,
        .location_group = loc_grp,
        .rgn_stack      = stack_create(),
        .rgn_defs       = queue_create(),
        .rgn_defs_stack = stack_create(),
        .attributes     = OTF2_AttributeList_New()
    };

    new->evt_writer = OTF2_Archive_GetEvtWriter(Archive, new->ref);
    new->def_writer = OTF2_Archive_GetDefWriter(Archive, new->ref);

    /* Thread location definition is written at thread-end (once all events
       counted) */

    LOG_DEBUG("[t=%lu] location created", id);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_stack:",      new->rgn_stack);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_defs:",       new->rgn_defs);
    LOG_DEBUG("[t=%lu] %-18s %p", id, "rgn_defs_stack:", new->rgn_defs_stack);

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
        .attr.parallel = {
            .id            = id,
            .master_thread = master,
            .is_league     = flags & ompt_parallel_league ? true : false,
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

trace_region_def_t *
trace_new_sync_region(
    trace_location_def_t *loc,
    ompt_sync_region_t    stype, 
    unique_id_t           encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = SYNC_TYPE_TO_OTF2_REGION_ROLE(stype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_synchronise,
        .encountering_task_id = encountering_task_id,
        .attr.sync = {
            .type = stype,
        }
    };

    LOG_DEBUG("[t=%lu] created sync region %u at %p",
        loc->id, new->ref, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;
}

trace_region_def_t *
trace_new_task_region(
    trace_location_def_t  *loc, 
    trace_region_def_t    *parent_task_region, 
    unique_id_t            id,
    ompt_task_flag_t       flags,
    int                    has_dependences)
{
    /* Create a region representing a task. Add to the location's region
       definition queue. */

    LOG_INFO_IF((parent_task_region == NULL),
        "[t=%lu] parent task region is null", loc->id);

    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref = get_unique_rgn_ref(),
        .role = OTF2_REGION_ROLE_TASK,
        .attributes = OTF2_AttributeList_New(),
        .type = trace_region_task,
        .attr.task = {
            .id              = id,
            .type            = flags & 0xF,
            .flags           = flags,
            .has_dependences = has_dependences,
            .parent_id   = parent_task_region != NULL ? 
                parent_task_region->attr.task.id   : OTF2_UNDEFINED_UINT64,
            .parent_type = parent_task_region != NULL ? 
                parent_task_region->attr.task.type : OTF2_UNDEFINED_UINT32,
            .task_status     = 0 /* no status */
        }
    };
    new->encountering_task_id = new->attr.task.parent_id;

    LOG_DEBUG("[t=%lu] created region %u for task %lu at %p",
        loc->id, new->ref, new->attr.task.id, new);

    /* Add region definition to location's region definition queue */
    queue_push(loc->rgn_defs, (data_item_t) {.ptr = new});

    return new;
}

/* * * * * * * * * * * * * * * */
/* * * * * Destructors * * * * */
/* * * * * * * * * * * * * * * */

void 
trace_destroy_location(trace_location_def_t *loc)
{
    if (loc == NULL) return;
    trace_write_location_definition(loc);
    LOG_DEBUG("[t=%lu] destroying rgn_stack %p", loc->id, loc->rgn_stack);
    stack_destroy(loc->rgn_stack, false, NULL);
    if (loc->rgn_defs)
    {
        LOG_DEBUG("[t=%lu] destroying rgn_defs %p", loc->id, loc->rgn_defs);
        queue_destroy(loc->rgn_defs, false, NULL);
    }
    LOG_DEBUG("[t=%lu] destroying rgn_defs_stack %p", loc->id, loc->rgn_defs_stack);
    stack_destroy(loc->rgn_defs_stack, false, NULL);
    // OTF2_AttributeList_Delete(loc->attributes);
    LOG_DEBUG("[t=%lu] destroying location", loc->id);
    free(loc);
    return;
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

    /* Lock the global def writer first */
    pthread_mutex_lock(&lock_global_def_writer);
    
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

#if defined(USE_OMPT_MASKED)
        case trace_region_masked:
#else
        case trace_region_master:
#endif
            trace_destroy_master_region(r);
            break;
        
        case trace_region_synchronise:
            trace_destroy_sync_region(r);
            break;

        case trace_region_task:
            trace_destroy_task_region(r);
            break;
        
        default:
            LOG_ERROR("unknown region type %d", r->type);
            abort();
        }
    }

    /* Release once done */
    pthread_mutex_unlock(&lock_global_def_writer);

    /* destroy parallel region once all locations are done with it
       and all definitions written */
    // OTF2_AttributeList_Delete(rgn->attributes);
    queue_destroy(rgn->attr.parallel.rgn_defs, false, NULL);
    LOG_DEBUG("region %p (parallel id %lu)", rgn, rgn->attr.parallel.id);
    free(rgn);
    return;
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
trace_destroy_master_region(trace_region_def_t *rgn)
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
        (!(rgn->attr.task.task_status == ompt_task_complete 
            || rgn->attr.task.task_status == ompt_task_cancel)),
        "destroying task region before task-complete/task-cancel");
    LOG_DEBUG("region %p destroying attribute list %p", rgn, rgn->attributes);
    OTF2_AttributeList_Delete(rgn->attributes);
    LOG_DEBUG("region %p", rgn);
    free(rgn);
}

/* pretty-print region definitions */
void
trace_region_pprint(
    FILE                *fp,
    trace_region_def_t  *r,
    const char func[],
    const int line)
{
    if (fp == NULL)
        fp = stderr;

    switch (r->type)
    {
    case trace_region_parallel:
        fprintf(fp, "%s:%d: Parallel(id=%lu, master=%lu, ref_count=%u, enter_count=%u) in %s:%d\n",
            __func__, __LINE__,
            r->attr.parallel.id,
            r->attr.parallel.master_thread,
            r->attr.parallel.ref_count,
            r->attr.parallel.enter_count,
            func, line
        );
        break;
    case trace_region_workshare:
        fprintf(fp, "%s:%d: Work(type=%s, count=%lu) in %s:%d\n",
            __func__, __LINE__,
            OMPT_WORK_TYPE_TO_STR(r->attr.wshare.type),
            r->attr.wshare.count,
            func, line
        );
        break;
    case trace_region_synchronise:
        fprintf(fp, "%s:%d: Sync(type=%s) in %s:%d\n",
            __func__, __LINE__,
            OMPT_SYNC_TYPE_TO_STR(r->attr.sync.type),
            func, line
        );
        break;
    case trace_region_task:
        fprintf(fp, "%s:%d: Task(id=%lu, type=%s) in %s:%d\n",
            __func__, __LINE__,
            r->attr.task.id,
            OMPT_TASK_TYPE_TO_STR(OMPT_TASK_TYPE_BITS & r->attr.task.type),
            func, line
        );
        break;
#if defined(USE_OMPT_MASKED)
    case trace_region_masked:
        fprintf(fp, "%s:%d: Masked(thread=%lu) in %s:%d\n",
#else
    case trace_region_master:
        fprintf(fp, "%s:%d: Master(thread=%lu) in %s:%d\n",
#endif
            __func__, __LINE__,
            r->attr.master.thread,
            func, line
        );
        break;
    }
    return;
}