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

#include "task-manager/one-to-queue.h"
#define trace_task_manager_alloc       trace_task_manager_one_to_queue_alloc
#define trace_task_manager_free        trace_task_manager_one_to_queue_free
#define trace_task_manager_add_task    trace_task_manager_one_to_queue_add_task
#define trace_task_manager_get_task    trace_task_manager_one_to_queue_get_task
#define trace_task_manager_pop_task    trace_task_manager_one_to_queue_pop_task
#define trace_task_manager_borrow_task trace_task_manager_one_to_queue_borrow_task

#endif // OTTER_TRACE_TASK_MANAGER_PUBLIC_H
