/**
 * @file otter-trace/task-manager/one-to-one.h
 * @author Adam Tuft
 * @brief Compile against the task-manager which maps a string to a single
 * task handle.
 * 
 * This implementation silently overwrites when a key is updated
 * 
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "api/otter-task-graph/otter-task-graph.h" // for otter_task_context typedef

typedef struct trace_task_manager_t trace_task_manager_t;

trace_task_manager_t* trace_task_manager_one_to_one_alloc(void);
void trace_task_manager_one_to_one_free(trace_task_manager_t*);
void trace_task_manager_one_to_one_add_task(trace_task_manager_t*, const char*, otter_task_context*);
otter_task_context* trace_task_manager_one_to_one_get_task(trace_task_manager_t*, const char*);
otter_task_context* trace_task_manager_one_to_one_pop_task(trace_task_manager_t*, const char*);
