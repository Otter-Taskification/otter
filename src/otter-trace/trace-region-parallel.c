#include <stdlib.h>
#include <pthread.h>
#include "src/otter-trace/trace-lookup-macros.h"
#include "src/otter-trace/trace-attributes.h"
#include "src/otter-trace/trace-archive.h"
#include "public/otter-trace/trace-region-parallel.h"
#include "src/otter-trace/trace-unique-refs.h"
#include "src/otter-trace/trace-check-error-code.h"
#include "public/types/queue.h"
#include "public/types/stack.h"

/* Defined in trace-archive.c */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

/* Defined in trace.c */
// extern pthread_mutex_t lock_global_def_writer;

/* Constructor */
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

/* Destructor */
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

/* Add parallel region attributes */
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
