/**
 * @file otter-task-context.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Defines the interface for manipulating the `otterTaskContext` struct.
 * Used by code which needs to manipulate this struct i.e. the otter-task-graph
 * event source. Not for use directly in user code.
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 */

#include "otter/otter-common.h"
#include "otter/otter-task-context.h"

/**
 * @brief Allocate an uninitialised otterTaskContext.
 * 
 * @return otterTaskContext* 
 */
otterTaskContext *otterTaskContext_alloc(void);

/**
 * @brief Initialise task as a child of parent. If parent is NULL, task is an
 * orphan i.e. with undefined parent.
 * 
 * @param task The task to initialise. Must not be NULL.
 * @param parent The parent of `task`, or NULL if `task` has no parent.
 */
void otterTaskContext_init(otterTaskContext *task, otterTaskContext *parent);

/**
 * @brief Delete a task context.
 * 
 * @param task The context to delete.
 */
void otterTaskContext_delete(otterTaskContext *task);

// Getters

/**
 * @brief Get a task context's ID
 * 
 * @param task The task to inspect.
 * @return unique_id_t 
 */
unique_id_t otterTaskContext_get_task_context_id(otterTaskContext *task);

/**
 * @brief Get the ID of the parent of a task context
 * 
 * @param task The task to inspect.
 * @return unique_id_t 
 */
unique_id_t otterTaskContext_get_parent_task_context_id(otterTaskContext *task);

/**
 * @brief Get the attribute list belonging to a task.
 * 
 * @param task The task whose attribute list should be returned.
 * @return OTF2_AttributeList* 
 */
OTF2_AttributeList *otterTaskContext_get_attribute_list(otterTaskContext *task);

// Assign IDs

/**
 * @brief Assign a new unique ID for a task context. Counts upwards by 1 each
 * time.
 * 
 * @return unique_id_t 
 */
unique_id_t otterTaskContext_get_unique_id(void);
