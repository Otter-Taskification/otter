/**
 * @file otter-task-context.c
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Implementation of otter_task_context opaque struct.
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "otf2/otf2.h"
#include "otter/otter-version.h"
#include "otter/general.h"
#include "otter/debug.h"
#include "otter/otter-common.h"
#include "otter/otter-task-context-interface.h"

#define TASK_ID_UNDEFINED        OTF2_UNDEFINED_UINT64

struct otter_task_context
{
    unique_id_t          task_context_id;
    unique_id_t          parent_task_context_id;
    uint64_t             task_create_time;
    uint64_t             task_start_time;
    uint64_t             task_end_time;
    OTF2_AttributeList  *attributes;
};

otter_task_context *otterTaskContext_alloc(void)
{
    LOG_DEBUG("allocate task context");
    return malloc(sizeof(otter_task_context));
}

void otterTaskContext_init(otter_task_context *task, otter_task_context *parent)
{
    assert(task != NULL);
    task->task_context_id = otterTaskContext_get_unique_id();
    task->attributes = OTF2_AttributeList_New();
    if (parent == NULL) {
        task->parent_task_context_id = TASK_ID_UNDEFINED;
    } else {
        task->parent_task_context_id = otterTaskContext_get_task_context_id(parent);
    }
    LOG_DEBUG("initialised task context: %lu", task->task_context_id);
}

void otterTaskContext_delete(otter_task_context *task)
{
    LOG_DEBUG("delete task context: %lu", task->task_context_id);
    OTF2_AttributeList_Delete(task->attributes);
    free(task);
}

// Getters

unique_id_t otterTaskContext_get_task_context_id(otter_task_context *task)
{
    // ! HACK ! only here to temporarily work around ExaHyPE not managing task pointers
    // assert(task != NULL);
    return task==NULL ? 0 : task->task_context_id;
}

unique_id_t otterTaskContext_get_parent_task_context_id(otter_task_context *task)
{
    // assert(task != NULL);
    return task==NULL ? 0 :  task->parent_task_context_id;
}

OTF2_AttributeList *otterTaskContext_get_attribute_list(otter_task_context *task)
{
    // assert(task != NULL);
    return task==NULL ? OTF2_AttributeList_New() : task->attributes;
}

// Assign IDs

unique_id_t otterTaskContext_get_unique_id(void)
{
    static unique_id_t ID=0;
    return __sync_fetch_and_add(&ID, 1L);
}
