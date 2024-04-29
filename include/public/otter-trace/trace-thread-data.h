#ifndef OTTER_TRACE_THREAD_DATA_H
#define OTTER_TRACE_THREAD_DATA_H

#include "api/otter-task-graph/otter-task-graph.h" // for otter_task_context typedef
#include "public/otter-common.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-state.h"
#include "public/otter-trace/trace-types.h"

// TODO: can this struct be made opaque?
typedef struct thread_data_t {
    unique_id_t id;
    trace_location_def_t *location;
    otter_thread_t type;
    bool is_master_thread; // of parallel region

    /**
     * @brief Records the handle corresponding to the task being executed by the thread. NULL means
     * the task is not currently executing an annotated task i.e. it is in the implicit task.
     *
     * Invariants:
     *  - at any point, this thread-local variable represents the handle of the task presently being
     *      executed.
     *  - if NULL, no task is being executed.
     *  - when the task being executed is changed, the thread must be notified and the active task
     *      updated.
     *  - the thread does *not* remember any other task, such as a task switched away from, or a parent
     *      task.
     *
     */
    otter_task_context *active_task;
} thread_data_t;

thread_data_t *new_thread_data(otter_thread_t type);

void thread_destroy(thread_data_t *thread_data);

#endif // OTTER_TRACE_THREAD_DATA_H
