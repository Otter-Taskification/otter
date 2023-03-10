/**
 * @file otter-task-graph.c
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Implementation of Otter task graph event source API for recording task graph via annotations
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 */

#define __USE_POSIX // HOST_NAME_MAX
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "otter/otter-version.h"
#include "otter/general.h"
#include "otter/debug.h"
#include "otter/otter-environment-variables.h"
#include "otter/trace.h"
#include "otter/otter-structs.h"
#include "otter/otter-task-graph.h"
#include "otter/otter-task-context-interface.h"

#define LOG_EVENT_CALL(file, func, line, ifunc) LOG_DEBUG("%s:%d in %s", file, line, func)

/* detect environment variables */
static otter_opt_t opt = {
    .hostname         = NULL,
    .tracename        = NULL,
    .tracepath        = NULL,
    .archive_name     = NULL,
    .append_hostname  = false
};

void otterTraceInitialise(void)
{
    // Initialise archive    

    static char host[HOST_NAME_MAX+1] = {0};
    gethostname(host, HOST_NAME_MAX);

    opt.hostname = host;
    opt.tracename = getenv(ENV_VAR_TRACE_OUTPUT);
    opt.tracepath = getenv(ENV_VAR_TRACE_PATH);
    opt.append_hostname = getenv(ENV_VAR_APPEND_HOST) == NULL ? false : true;

    /* Apply defaults if variables not provided */
    if(opt.tracename == NULL) opt.tracename = DEFAULT_OTF2_TRACE_OUTPUT;
    if(opt.tracepath == NULL) opt.tracepath = DEFAULT_OTF2_TRACE_PATH;

    LOG_INFO("Otter environment variables:");
    LOG_INFO("%-30s %s", "host", opt.hostname);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_PATH,   opt.tracepath);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_OUTPUT, opt.tracename);
    LOG_INFO("%-30s %s", ENV_VAR_APPEND_HOST,  opt.append_hostname?"Yes":"No");

    trace_initialise_archive(&opt);

    // Write the definition of a dummy location
    // trace_write_location_definition(...)? or simply via OTF2_GlobalDefWriter_WriteLocation(...)

    return;
}

void otterTraceFinalise(void)
{
    // Finalise arhchive
    LOG_DEBUG("=== finalising archive ===");

    // Ensure a single location definition is written to the archive
    thread_data_t *dummy_thread = new_thread_data(otter_thread_initial);
    thread_destroy(dummy_thread);

    trace_finalise_archive();
    char trace_folder[PATH_MAX] = {0};
    realpath(opt.tracepath, &trace_folder[0]);
    fprintf(stderr, "%s%s/%s\n",
        "OTTER_TRACE_FOLDER=", trace_folder, opt.archive_name);
    return;
}

void otterTraceStart(void)
{
    LOG_DEBUG("not currently implemented - ignored");
}

void otterTraceStop(void)
{
    LOG_DEBUG("not currently implemented - ignored");
}

otter_task_context *otterTaskBegin(const char* file, const char* func, int line, otter_task_context *parent)
{
    otter_task_context *task = otterTaskContext_alloc();
    otterTaskContext_init(task, parent);
    trace_region_attr_t task_attr;
    task_attr.task.type = otter_task_explicit;
    task_attr.task.id = otterTaskContext_get_task_context_id(task);
    task_attr.task.parent_id = parent==NULL ? (unique_id_t) (~0) : otterTaskContext_get_task_context_id(parent);
    LOG_DEBUG("[%lu] begin task (child of %lu)", task_attr.task.id, task_attr.task.parent_id);
    trace_graph_event_task_begin(task, task_attr);
    return task;
}

void otterTaskEnd(otter_task_context *task)
{
    LOG_DEBUG("[%lu] end task", otterTaskContext_get_task_context_id(task));
    trace_graph_event_task_end(task);
    otterTaskContext_delete(task);
}

void otterSynchroniseTasks(otter_task_context *task, otter_task_sync_t mode)
{
    LOG_DEBUG("synchronise tasks: %d", mode);
    trace_region_attr_t sync_attr;
    sync_attr.sync.type = otter_sync_region_taskwait;
    sync_attr.sync.sync_descendant_tasks = mode == otter_sync_descendants ? true : false;
    sync_attr.sync.encountering_task_id = otterTaskContext_get_task_context_id(task);
    trace_graph_synchronise_tasks(task, sync_attr);
    return;
}
