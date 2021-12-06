#include <stdint.h>
#include <stdlib.h>
// #include "otter/otter-ompt-header.h"
#include "otter/otter.h"
#include "otter/otter-structs.h"
#include "otter/trace.h"
#include "otter/trace-structs.h"

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
    int                   has_dependences)
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
        has_dependences
    );
    return new;
}

void task_destroy(task_data_t *task_data)
{
    free(task_data);
    return;
}

unique_id_t
get_unique_id(unique_id_type_t id_type)
{
    static unique_id_t id[NUM_ID_TYPES] = {0,0,0,0};
    return __sync_fetch_and_add(&id[id_type], 1L);
}
