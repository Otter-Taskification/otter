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
#include <limits.h>
#include <otf2/otf2.h>
#include "public/otter-version.h"
#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-trace/trace-task-context-interface.h"

#define TASK_ID_UNDEFINED        OTF2_UNDEFINED_UINT64

struct otter_task_context
{
    unique_id_t          task_context_id;
    unique_id_t          parent_task_context_id;
    uint64_t             task_create_time;
    uint64_t             task_start_time;
    uint64_t             task_end_time;
    int                  flavour;
    otter_src_ref_t      init_location;
};

otter_task_context *otterTaskContext_alloc(void)
{
    LOG_DEBUG("allocate task context");
    return malloc(sizeof(otter_task_context));
}

void otterTaskContext_init(otter_task_context *task, otter_task_context *parent, int flavour, otter_src_ref_t init_location)
{
    assert(task != NULL);
    static unique_id_t unique_id=0;
    task->task_context_id = __sync_fetch_and_add(&unique_id, 1L);
    task->flavour = flavour;
    task->init_location = init_location;
    if (parent == NULL) {
        task->parent_task_context_id = TASK_ID_UNDEFINED;
    } else {
        task->parent_task_context_id = parent->task_context_id;
    }
    LOG_DEBUG("initialised task context: %lu", task->task_context_id);
}

void otterTaskContext_delete(otter_task_context *const task)
{
    LOG_DEBUG("delete task context: %lu", task->task_context_id);
    free(task);
}

// Getters

unique_id_t otterTaskContext_get_task_context_id(const otter_task_context *task)
{
    // ! HACK ! only here to temporarily work around ExaHyPE not managing task pointers
    // assert(task != NULL);
    return task==NULL ? 0 : task->task_context_id;
}

unique_id_t otterTaskContext_get_parent_task_context_id(const otter_task_context *task)
{
    // assert(task != NULL);
    return task==NULL ? 0 :  task->parent_task_context_id;
}

int otterTaskContext_get_task_flavour(const otter_task_context *task)
{
    return task==NULL ? INT_MAX : task->flavour;
}

otter_src_ref_t otterTaskContext_get_init_location_ref(const otter_task_context *task)
{
    return task==NULL ? (otter_src_ref_t){0,0,0} : task->init_location;
}
