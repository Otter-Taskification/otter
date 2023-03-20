#include <stdint.h>
#include <stdlib.h>
#include "private/otter-ompt/otter.h"
#include "public/types/otter-structs.h"
#include "public/otter-trace/trace.h"
#include "public/trace-structs.h"
#include "private/otter-trace/trace-static-constants.h"

/* Used as an array index to keep track of unique ids for different entities */
typedef enum {
    id_timestamp,
    id_parallel,
    id_thread,
    id_task,
    NUM_ID_TYPES
} unique_id_type_t;

static unique_id_t get_unique_id(unique_id_type_t id_type);

parallel_data_t *
new_parallel_data(
    unique_id_t  thread_id,
    unique_id_t  encountering_task_id,
    task_data_t *encountering_task_data,
    unsigned int requested_parallelism,
    int          flags)
{
    parallel_data_t *parallel_data = malloc(sizeof(*parallel_data));
    *parallel_data = (parallel_data_t) {
        .id                      = get_unique_parallel_id(),
        .master_thread           = thread_id,
        .encountering_task_data  = encountering_task_data,
        .region                  = NULL
    };

    parallel_data->region = trace_new_parallel_region(
        parallel_data->id,
        thread_id,
        encountering_task_id,
        flags,
        requested_parallelism);
    return parallel_data;
}

void parallel_destroy(parallel_data_t *parallel_data)
{
    free(parallel_data);
    return;
}

thread_data_t *
new_thread_data(otter_thread_t type)
{
    thread_data_t *thread_data = malloc(sizeof(*thread_data));
    *thread_data = (thread_data_t) {
        .id                 = get_unique_thread_id(),
        .location           = NULL,
        .type               = type,
        .is_master_thread   = false
    };

    /* Create a location definition for this thread */
    thread_data->location = trace_new_location_definition(
        thread_data->id,
        type,
        OTF2_LOCATION_TYPE_CPU_THREAD,
        DEFAULT_LOCATION_GRP);

    return thread_data;
}

void 
thread_destroy(thread_data_t *thread_data)
{
    trace_destroy_location(thread_data->location);
    free(thread_data);
}

task_data_t *
new_task_data(
    trace_location_def_t *loc,
    trace_region_def_t   *parent_task_region,
    unique_id_t           task_id,
    otter_task_flag_t     flags,
    int                   has_dependences,
    otter_src_location_t *src_location,
    const void           *task_create_ra)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id     = task_id,
        .type   = flags & OMPT_TASK_TYPE_BITS,
        .flags  = flags,
        .region = NULL
    };
    new->region = trace_new_task_region(
        loc, 
        parent_task_region, 
        new->id,
        flags, 
        has_dependences,
        src_location,
        task_create_ra
    );
    return new;
}

void task_destroy(task_data_t *task_data)
{
    free(task_data);
    return;
}

unique_id_t get_unique_parallel_id(void)
{
    return get_unique_id(id_parallel);
}

unique_id_t get_unique_thread_id(void)
{
    return get_unique_id(id_thread);
}

unique_id_t get_unique_task_id(void)
{
    return get_unique_id(id_task);
}

unique_id_t
get_unique_id(unique_id_type_t id_type)
{
    static unique_id_t id[NUM_ID_TYPES] = {0,0,0,0};
    return __sync_fetch_and_add(&id[id_type], 1L);
}
