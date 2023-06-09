#include "public/debug.h"

#include "public/otter-trace/task-manager/one-to-queue.h"
#include "public/types/vptr_manager.hpp"
#include "public/types/queue.h"

#include "public/otter-trace/trace-task-context-interface.h"

/**
 * @brief Invariants:
 * 
 *  - the task pointers represent valid tasks
 *  - a null task is never added to any queue in the manager
 *  - a queue held in the manager may be empty
 * 
 */

trace_task_manager_t* trace_task_manager_one_to_queue_alloc(void) {
    LOG_DEBUG("allocating task manager");
    trace_task_manager_t* manager = (trace_task_manager_t*) vptr_manager_make();
    LOG_DEBUG("allocated task manager: %p", manager);
    return manager;
}

void trace_task_manager_one_to_queue_free(trace_task_manager_t* manager, void(*callback)(const char*, int)) {
    LOG_DEBUG("freeing task manager: %p", manager);
    // TODO: clean up any queues left in the manager
    vptr_manager_delete((vptr_manager*) manager, callback);
}

void trace_task_manager_one_to_queue_add_task(trace_task_manager_t* manager, const char* task_key, otter_task_context* task) {

    if (task == NULL) return;

    LOG_DEBUG("adding task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );

    // get (or create) the queue for this key
    otter_queue_t* task_queue = (otter_queue_t*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    if (task_queue == NULL) {
        task_queue = queue_create();
    }

    // add the task to this queue WITHOUT checking whether it already exists there
    queue_push(task_queue, (data_item_t) {.ptr = task});

    // ensure the manager holds the queue under this key
    vptr_manager_insert_item((vptr_manager*) manager, task_key, (void*) task_queue);
}

otter_task_context* trace_task_manager_one_to_queue_get_task(trace_task_manager_t* manager, const char* task_key) {

#if 1
    LOG_ERROR("this method not yet supported! TODO: implement queue_peek(otter_queue_t*, data_item_t*)");
    return NULL;
#else
    // get the queue for this key. If no queue, warn and return NULL
    otter_queue_t* task_queue = (otter_queue_t*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    if (task_queue == NULL) {
        LOG_WARN("(manager=%p) no tasks found for key: '%s'", manager, task_key);
        return NULL;
    }

    // get a task from the queue (without popping it) and return it.
    otter_task_context* task = NULL; // TODO: implement queue_peek(...)

    LOG_DEBUG("got task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );
    return task;
#endif
}

otter_task_context* trace_task_manager_one_to_queue_pop_task(trace_task_manager_t* manager, const char* task_key) {

    // get the queue for this key. If no queue, warn and return NULL
    otter_queue_t* task_queue = (otter_queue_t*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    if (task_queue == NULL) {
        LOG_WARN("(manager=%p) no task queue found for key: '%s'", manager, task_key);
        return NULL;
    }

    // pop a task from the queue and return it.
    otter_task_context* task = NULL;
    if (!queue_is_empty(task_queue)) {
        queue_pop(task_queue, (data_item_t*) &task);
    }

    LOG_DEBUG("got task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );

    return task;
}

otter_task_context* trace_task_manager_one_to_queue_borrow_task(trace_task_manager_t* manager, const char* task_key) {

    // get the queue for this key. If no queue, warn and return NULL
    otter_queue_t* task_queue = (otter_queue_t*) vptr_manager_get_item((vptr_manager*) manager, task_key);
    if (task_queue == NULL) {
        LOG_WARN("(manager=%p) no task queue found for key: '%s'", manager, task_key);
        return NULL;
    }

    // borrow the next task by peeking at the front of the queue. Do not pop.
    otter_task_context* task = NULL;
    if (!queue_is_empty(task_queue)) {
        queue_peek(task_queue, (data_item_t*) &task);
    }

    LOG_DEBUG("got task (manager=%p, task=%p, task_unique_id=%lu, task_key='%s'",
        manager,
        task,
        otterTaskContext_get_task_context_id(task),
        task_key
    );

    return task;
}
