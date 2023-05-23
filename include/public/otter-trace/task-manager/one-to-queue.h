/**
 * @file otter-trace/task-manager/one-to-one.h
 * @author Adam Tuft
 * @brief Compile against the task-manager which maps a string to a queue of
 * task handles.
 * 
 * This implementation appends a handle to the corresponding queue each time a
 * key is updated. No warning is given if the same task handle is inserted
 * multiple times.
 * 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "api/otter-task-graph/otter-task-graph.h" // for otter_task_context typedef

typedef struct trace_task_manager_t trace_task_manager_t;

trace_task_manager_t* trace_task_manager_one_to_queue_alloc(void);
void trace_task_manager_one_to_queue_free(trace_task_manager_t*);
void trace_task_manager_one_to_queue_add_task(trace_task_manager_t*, const char*, otter_task_context*);
otter_task_context* trace_task_manager_one_to_queue_get_task(trace_task_manager_t*, const char*);
otter_task_context* trace_task_manager_one_to_queue_pop_task(trace_task_manager_t*, const char*);
otter_task_context* trace_task_manager_one_to_queue_borrow_task(trace_task_manager_t*, const char*);
