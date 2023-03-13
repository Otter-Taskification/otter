/**
 * @file otter-task-context.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Header used by user code which only needs to know of the existence of
 * the otter_task_context type.
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 */

/**
 * @brief Declares the existence of the otter_task_context struct.
 * 
 * This struct is an opaque representation of a task context used by the 
 * `otter-task-graph.h` tracing API. The definition of this struct is hidden 
 * from the user as an implementation detail.
 * 
 */
typedef struct otter_task_context otter_task_context;
