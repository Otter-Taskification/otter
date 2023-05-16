#include "public/debug.h"

#include "public/otter-trace/task-manager/one-to-one.h"
#include "public/types/vptr_manager.hpp"

#include "public/otter-trace/trace-task-context-interface.h"

trace_task_manager_t* trace_task_manager_one_to_one_alloc(void) {
    LOG_DEBUG("allocating task manager");
    trace_task_manager_t* manager = (trace_task_manager_t*) vptr_manager_make();
    LOG_DEBUG("allocated task manager: %p", manager);
    return manager;
}

void trace_task_manager_one_to_one_free(trace_task_manager_t* manager) {
    LOG_DEBUG("freeing task manager: %p", manager);
    vptr_manager_delete((vptr_manager*) manager);
}

void trace_task_manager_one_to_one_add_task(trace_task_manager_t* manager, const char* task_key, otter_task_context* task) {
    LOG_DEBUG("adding task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );
    vptr_manager_insert_item((vptr_manager*) manager, task_key, (void*) task);
}

otter_task_context* trace_task_manager_one_to_one_get_task(trace_task_manager_t* manager, const char* task_key) {
    otter_task_context* task = (otter_task_context*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    LOG_DEBUG("got task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );
    return task;
}

otter_task_context* trace_task_manager_one_to_one_pop_task(trace_task_manager_t* manager, const char* task_key) {
    otter_task_context* task = (otter_task_context*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    LOG_DEBUG("got task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );
    return task;
}
