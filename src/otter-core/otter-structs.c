#include <stdint.h>
#include <stdlib.h>
#include <otter-ompt-header.h>
#include <otter-core/otter.h>
#include <otter-core/otter-structs.h>
#include <otter-task-graph/task-graph.h>
#include <otter-trace/trace.h>

parallel_data_t *
new_parallel_data(int flags)
{
    parallel_data_t *parallel_data = malloc(sizeof(*parallel_data));
    *parallel_data = (parallel_data_t) {
        .id     = get_unique_parallel_id(),
        .region = NULL
    };
    return parallel_data;
}

void parallel_destroy(parallel_data_t *parallel_data)
{
    free(parallel_data);
    return;
}

thread_data_t *
new_thread_data(ompt_thread_t type)
{
    thread_data_t *thread_data = malloc(sizeof(*thread_data));
    *thread_data = (thread_data_t) {
        .id                 = get_unique_thread_id(),
        .location           = NULL,
        .type               = type,
        .is_master_thread   = false
    };
    return thread_data;
}

void 
thread_destroy(thread_data_t *thread_data)
{
    free(thread_data);
}

task_data_t *
new_task_data(
    unique_id_t      id,
    ompt_task_flag_t flags)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id     = id,
        .type   = flags & OMPT_TASK_TYPE_BITS,
        .flags  = flags,
        .region = NULL
    };
    return new;
}

void task_destroy(task_data_t *task_data)
{
    free(task_data);
    return;
}
