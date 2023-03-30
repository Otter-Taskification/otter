#include <stdlib.h>
#include "public/otter-trace/trace-parallel-data.h"
#include "public/otter-trace/trace-region-def.h"
#include "otter-trace/trace-get-unique-id.h"

typedef struct parallel_data_t {
    unique_id_t         id;
    unique_id_t         master_thread;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
} parallel_data_t;

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
        .id                      = get_unique_id(),
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

task_data_t *
trace_parallel_get_task_data(parallel_data_t *parallel) {
    return parallel->encountering_task_data;
}

trace_region_def_t *
trace_parallel_get_region_def(parallel_data_t *parallel) {
    return parallel->region;
}
