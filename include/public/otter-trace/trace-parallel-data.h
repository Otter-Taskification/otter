#ifndef OTTER_TRACE_PARALLEL_DATA_H
#define OTTER_TRACE_PARALLEL_DATA_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-task-data.h"

// TODO: this struct is almost opaque - refactor any accesses into 
// TODO: trace-parallel-data.c and declare as public opaque type.
// TODO: need getters for region & encountering_task_data.
typedef struct parallel_data_t {
    unique_id_t         id;
    unique_id_t         master_thread;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
} parallel_data_t;

parallel_data_t *
new_parallel_data(
    unique_id_t thread_id,
    unique_id_t encountering_task_id,
    task_data_t *encountering_task_data,
    unsigned int requested_parallelism,
    int flags
);

void
parallel_destroy(
    parallel_data_t *parallel_data
);

#endif // OTTER_TRACE_PARALLEL_DATA_H
