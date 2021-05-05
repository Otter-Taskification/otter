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
        .id                      = get_unique_parallel_id(),
        .flags                   = flags,
        .actual_parallelism      = 0,
        .region                  = NULL,
        .scope                   = NULL
    };

    /* NOTE: don't push scope onto thread's stack UNTIL implicit-task-begin */
    parallel_data->scope = new_scope(scope_parallel, parallel_data);

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
        .id                          = get_unique_thread_id(),
        .location                    = NULL,
        .type                        = type,
        .region_scope_stack          = stack_create(),
        .is_master_thread            = false,
        .prior_scope                 = NULL,
        .sync_node_queue             = queue_create()
    };
    return thread_data;
}

void 
thread_destroy(thread_data_t *thread_data)
{
    /* A thread doesn't own the data in its scope stack and node queue */
    queue_destroy(thread_data->sync_node_queue, false, NULL);
    stack_destroy(thread_data->region_scope_stack, false, NULL);
    free(thread_data);
}

task_data_t *
new_task_data(
    unique_id_t      id,
    ompt_task_flag_t flags)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id         = id,
        .type       = flags & OMPT_TASK_TYPE_BITS,
        .flags      = flags
    };
    return new;
}

void task_destroy(task_data_t *task_data)
{
    free(task_data);
    return;
}

scope_t *
new_scope(
    scope_type_t  type,
    void    *data)
{
    scope_t *scope = malloc(sizeof(*scope));
    *scope = (scope_t) {
        .type = type,
        .endpoint = ompt_scope_begin,
        .data = data,
        .task_graph_nodes = stack_create(),
        .begin_node = task_graph_add_node(
            type == scope_parallel       ? node_scope_parallel_begin       :
            type == scope_sections       ? node_scope_sections_begin       :
            type == scope_single         ? node_scope_single_begin         :
            type == scope_loop           ? node_scope_loop_begin           :
            type == scope_taskloop       ? node_scope_taskloop_begin       :
            type == scope_sync_taskgroup ? node_scope_sync_taskgroup_begin :
                node_type_unknown,
            (task_graph_node_data_t) {.ptr = data}),
        .end_node = task_graph_add_node(
            type == scope_parallel       ? node_scope_parallel_end         :
            type == scope_sections       ? node_scope_sections_end         :
            type == scope_single         ? node_scope_single_end           :
            type == scope_loop           ? node_scope_loop_end             :
            type == scope_taskloop       ? node_scope_taskloop_end         :
            type == scope_sync_taskgroup ? node_scope_sync_taskgroup_end   :
                node_type_unknown,
            (task_graph_node_data_t) {.ptr = data}),
        .lock = PTHREAD_MUTEX_INITIALIZER
    };
    return scope;
}

void
scope_destroy(scope_t *scope)
{
    /* A scope does not own the nodes in its stack - don't destroy them */
    stack_destroy(scope->task_graph_nodes, false, NULL);
    free(scope);
    return;
}
