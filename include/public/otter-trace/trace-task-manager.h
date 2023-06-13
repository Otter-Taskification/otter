/**
 * @file trace-task-manager.h
 * @author Adam Tuft
 * @brief Public header for trace-task-manager.c
 * @version 0.1
 * @date 2023-05-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_TASK_MANAGER_PUBLIC_H)
#define OTTER_TRACE_TASK_MANAGER_PUBLIC_H

#include "api/otter-task-graph/otter-task-graph.h" // for otter_task_context typedef

typedef struct trace_task_manager_t trace_task_manager_t;
typedef void(trace_task_manager_callback)(const char*, int, void*);

trace_task_manager_t* trace_task_manager_alloc(void);
void trace_task_manager_free(trace_task_manager_t*);
void trace_task_manager_add_task(trace_task_manager_t*, const char*, otter_task_context*);
otter_task_context* trace_task_manager_get_task(trace_task_manager_t*, const char*);
otter_task_context* trace_task_manager_pop_task(trace_task_manager_t*, const char*);
otter_task_context* trace_task_manager_borrow_task(trace_task_manager_t*, const char*);
void trace_task_manager_count_insertions(trace_task_manager_t*, trace_task_manager_callback*, void*);

#endif // OTTER_TRACE_TASK_MANAGER_PUBLIC_H
