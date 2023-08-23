/**
 * @file trace-task-context-interface.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Defines the interface for manipulating the `otter_task_context`
 * struct. Used by code which needs to manipulate this struct i.e. the
 * otter-task-graph event source. Not for use directly in user code.
 * @version 0.2.0
 * @date 2022-10-03
 *
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 *
 */

#pragma once

#include "public/config.h"

#include "api/otter-task-graph/otter-task-graph.h" // for otter_task_context typedef
#include "public/otter-common.h"

/**
 * @brief Allocate an uninitialised otter_task_context.
 *
 * @return otter_task_context*
 */
otter_task_context *otterTaskContext_alloc(void);

/**
 * @brief Initialise task as a child of parent. If parent is NULL, task is an
 * orphan i.e. with undefined parent.
 *
 * @param task The task to initialise. Must not be NULL.
 * @param parent The parent of `task`, or NULL if `task` has no parent.
 * @param flavour The flavour of the new task.
 */
void otterTaskContext_init(otter_task_context *task, otter_task_context *parent,
                           int flavour, otter_src_ref_t init_location);

/**
 * @brief Delete a task context.
 *
 * @param task The context to delete.
 */
void otterTaskContext_delete(otter_task_context *task);

// Getters

/**
 * @brief Get a task context's ID
 *
 * @param task The task to inspect.
 * @return unique_id_t
 */
unique_id_t
otterTaskContext_get_task_context_id(const otter_task_context *task);

/**
 * @brief Get the ID of the parent of a task context
 *
 * @param task The task to inspect.
 * @return unique_id_t
 */
unique_id_t
otterTaskContext_get_parent_task_context_id(const otter_task_context *task);

/**
 * @brief Get the flavour of a task
 *
 * @param task The task to inspect.
 * @return int
 */
int otterTaskContext_get_task_flavour(const otter_task_context *task);

/**
 * @brief Get the source location where a task was initialised
 *
 */
otter_src_ref_t
otterTaskContext_get_init_location_ref(const otter_task_context *task);

/**
 * @brief Get the label associated with this task, if any.
 *
 */
otter_string_ref_t
otterTaskContext_get_task_label_ref(const otter_task_context *task);

/**
 * @brief Set the label a task was associated with.
 */
void otterTaskContext_set_task_label_ref(otter_task_context *task,
                                         otter_string_ref_t label);
