#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <otf2/otf2.h>
#include <otf2/OTF2_Pthread_Locks.h>

#include <macros/debug.h>
#include <otter-common.h>
#include <otter-trace/trace.h>
#include <otter-trace/trace-lookup-macros.h>
#include <otter-trace/trace-structs.h>

#include <otter-datatypes/queue.h>
#include <otter-datatypes/stack.h>

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
        .attributes     = OTF2_AttributeList_New()
    };

    new->evt_writer = OTF2_Archive_GetEvtWriter(Archive, new->ref);
    new->def_writer = OTF2_Archive_GetDefWriter(Archive, new->ref);

    /* Thread location definition is written at thread-end (once all events
       counted) */

    return new;
}

trace_region_def_t *
trace_new_parallel_region(
    unique_id_t           id, 
    unique_id_t           master, 
    int                   flags,
    unsigned int          requested_parallelism)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_PARALLEL,
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_parallel,
        .attr.parallel = {
            .id            = id,
            .master_thread = master,
            .is_league     = flags & ompt_parallel_league ? true : false,
            .requested_parallelism = requested_parallelism,
            .ref_count     = 0,
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
    uint64_t              count)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = WORK_TYPE_TO_OTF2_REGION_ROLE(wstype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_workshare,
        .attr.wshare = {
            .type       = wstype,
            .count      = count
        }
    };

    LOG_DEBUG("location %lu created region %u (workshare) at %p",
        loc->ref, new->ref, new);

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
        .attr.sync = {
            .type = stype,
            .encountering_task_id = encountering_task_id
        }
    };

    LOG_DEBUG("location %lu created region %u (sync) at %p",
        loc->ref, new->ref, new);

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

    LOG_INFO_IF((parent_task_region == NULL), "parent task region is null");

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
                parent_task_region->attr.task.id   : 0,
            .parent_type = parent_task_region != NULL ? 
                parent_task_region->attr.task.type : 0,
            .task_status     = 0 /* no status */
        }
    };

    LOG_DEBUG("location %lu created task region %u (sync) at %p",
        loc->ref, new->ref, new);

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
    LOG_DEBUG("destroying location %lu (Otter id %lu)", loc->ref, loc->id);
    LOG_DEBUG("%lu %lu", stack_size(loc->rgn_stack), queue_length(loc->rgn_defs));
    stack_destroy(loc->rgn_stack, false, NULL);
    queue_destroy(loc->rgn_defs, false, NULL);
    OTF2_AttributeList_Delete(loc->attributes);
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

    LOG_DEBUG("(%3d) destroying parallel region %u (Otter id %lu)",
        __LINE__, rgn->ref, rgn->attr.parallel.id);

    /* Lock the global def writer first */
    pthread_mutex_lock(&lock_global_def_writer);
    
    /* Write parallel region's definition */
    trace_write_region_definition(rgn);

    /* write region's nested region definitions */
    trace_region_def_t *r = NULL;
    while (queue_pop(rgn->attr.parallel.rgn_defs, (data_item_t*) &r))
    {
        trace_write_region_definition(r);

        /* destroy each region once its definition is written */
        switch (r->type)
        {
        case trace_region_workshare:
            trace_destroy_workshare_region(r);
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
    OTF2_AttributeList_Delete(rgn->attributes);
    queue_destroy(rgn->attr.parallel.rgn_defs, false, NULL);
    free(rgn);
    return;
}

void 
trace_destroy_workshare_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("(%3d) destroying workshare region %u (wstype %d)",
        __LINE__, rgn->ref, rgn->attr.wshare.type);
    OTF2_AttributeList_Delete(rgn->attributes);
    free(rgn);
}

void
trace_destroy_sync_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("(%3d) destroying sync region %u (sync type %d)",
        __LINE__, rgn->ref, rgn->attr.sync.type);
    OTF2_AttributeList_Delete(rgn->attributes);
    free(rgn);
}

void
trace_destroy_task_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("(%3d) destroying task region %u (Otter id %lu)",
        __LINE__, rgn->ref, rgn->attr.task.id);   
    OTF2_AttributeList_Delete(rgn->attributes);
    free(rgn);
}