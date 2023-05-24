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
#include <stdarg.h>

#include "public/otter-version.h"
#include "public/config.h"
#include "public/debug.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-initialise.h"
#include "public/otter-trace/trace-task-graph.h"
#include "api/otter-task-graph/otter-task-graph.h"
#include "public/otter-trace/trace-task-context-interface.h"
#include "public/otter-trace/trace-thread-data.h"
#include "public/otter-trace/trace-task-manager.h"
#include "public/otter-trace/source-location.h"

#define SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER 256

/* detect environment variables */
static otter_opt_t opt = {
    .hostname         = NULL,
    .tracename        = NULL,
    .tracepath        = NULL,
    .archive_name     = NULL,
    .append_hostname  = false
};

// TODO: move into trace_state_t
static pthread_mutex_t task_manager_mutex = PTHREAD_MUTEX_INITIALIZER;
static trace_task_manager_t *task_manager = NULL;
#define TASK_MANAGER_LOCK() pthread_mutex_lock(&task_manager_mutex)
#define TASK_MANAGER_UNLOCK() pthread_mutex_unlock(&task_manager_mutex)

static void otter_register_task_label_va_list(otter_task_context *task, const char *format, va_list args)
{
    char label_buffer[SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER] = {0};
    vsnprintf(&label_buffer[0], SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER, format, args);
    LOG_DEBUG("register task with label: %s", label_buffer);
    TASK_MANAGER_LOCK();
    trace_task_manager_add_task(task_manager, &label_buffer[0], task);
    TASK_MANAGER_UNLOCK();
}

void otterTraceInitialise(void)
{
    // Initialise archive    

    static char host[HOST_NAME_MAX+1] = {0};
    gethostname(host, HOST_NAME_MAX);

    opt.hostname = host;
    opt.tracename = getenv(ENV_VAR_TRACE_OUTPUT);
    opt.tracepath = getenv(ENV_VAR_TRACE_PATH);
    opt.append_hostname = getenv(ENV_VAR_APPEND_HOST) == NULL ? false : true;
    opt.event_model = otter_event_model_task_graph;

    /* Apply defaults if variables not provided */
    if(opt.tracename == NULL) opt.tracename = DEFAULT_OTF2_TRACE_OUTPUT;
    if(opt.tracepath == NULL) opt.tracepath = DEFAULT_OTF2_TRACE_PATH;

    LOG_INFO("Otter environment variables:");
    LOG_INFO("%-30s %s", "host", opt.hostname);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_PATH,   opt.tracepath);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_OUTPUT, opt.tracename);
    LOG_INFO("%-30s %s", ENV_VAR_APPEND_HOST,  opt.append_hostname?"Yes":"No");

    trace_initialise(&opt);
    task_manager = trace_task_manager_alloc();

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

    trace_finalise();
    trace_task_manager_free(task_manager);
    char trace_folder[PATH_MAX] = {0};
    realpath(opt.tracepath, &trace_folder[0]);
    fprintf(stderr, "%s%s/%s\n",
        "OTTER_TRACE_FOLDER:", trace_folder, opt.archive_name);
    return;
}

otter_task_context *otterTaskInitialise(otter_task_context *parent, int flavour, bool push_task, otter_source_args init, const char *format, ...)
{
    otter_task_context *task = otterTaskContext_alloc();
    otter_src_ref_t init_ref = get_source_location_ref((otter_src_location_t){
        .file = init.file,
        .func = init.func,
        .line = init.line
    });
    otterTaskContext_init(task, parent, flavour, init_ref);
    if (push_task) {
        va_list args;
        va_start(args, format);
        otter_register_task_label_va_list(task, format, args);
        va_end(args);
    }
    return task;
}

otter_task_context *otterTaskStart(otter_task_context *task, otter_source_args start)
{
    if (task == NULL) {
        LOG_ERROR("IGNORED (tried to start null task at %s:%d in %s)", start.file, start.line, start.func);
        return NULL;
    }
    trace_task_region_attr_t task_attr;
    task_attr.type = otter_task_explicit;
    task_attr.id = otterTaskContext_get_task_context_id(task);
    task_attr.parent_id = otterTaskContext_get_parent_task_context_id(task);
    task_attr.flavour = otterTaskContext_get_task_flavour(task);
    LOG_DEBUG("[%lu] begin task (child of %lu)", task_attr.id, task_attr.parent_id);
    otter_src_ref_t start_ref = get_source_location_ref((otter_src_location_t){
        .file = start.file, 
        .func = start.func,
        .line = start.line
    });
    trace_graph_event_task_begin(task, task_attr, start_ref);
    return task;
}

void otterTaskEnd(otter_task_context *task, otter_source_args end)
{
    LOG_DEBUG("[%lu] end task", otterTaskContext_get_task_context_id(task));
    otter_src_ref_t end_ref = get_source_location_ref((otter_src_location_t){
        .file = end.file, 
        .func = end.func,
        .line = end.line
    });
    trace_graph_event_task_end(task, end_ref);
    otterTaskContext_delete(task);
}

void otterTaskPushLabel(otter_task_context *task, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    otter_register_task_label_va_list(task, format, args);
    va_end(args);
    return;
}

otter_task_context *otterTaskPopLabel(const char *format, ...)
{
    char label_buffer[SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(&label_buffer[0], SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER, format, args);
    va_end(args);
    LOG_DEBUG("pop task with label: %s", label_buffer);
    TASK_MANAGER_LOCK();
    otter_task_context *task = trace_task_manager_pop_task(task_manager, label_buffer);
    TASK_MANAGER_UNLOCK();
    return task;
}

otter_task_context *otterTaskBorrowLabel(const char *format, ...)
{
    char label_buffer[SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(&label_buffer[0], SOME_LARGE_NUMBER_I_SHOULD_REDEFINE_LATER, format, args);
    va_end(args);
    LOG_DEBUG("pop task with label: %s", label_buffer);
    TASK_MANAGER_LOCK();
    otter_task_context *task = trace_task_manager_borrow_task(task_manager, label_buffer);
    TASK_MANAGER_UNLOCK();
    return task;
}

void otterSynchroniseTasks(otter_task_context *task, otter_task_sync_t mode)
{
    LOG_DEBUG("synchronise tasks: %d", mode);
    trace_sync_region_attr_t sync_attr;
    sync_attr.type = otter_sync_region_taskwait;
    sync_attr.sync_descendant_tasks = mode == otter_sync_descendants ? true : false;
    sync_attr.encountering_task_id = otterTaskContext_get_task_context_id(task);
    trace_graph_synchronise_tasks(task, sync_attr);
    return;
}

void otterTraceStart(void) {
    LOG_DEBUG("not currently implemented - ignored");
}

void otterTraceStop(void) {
    LOG_DEBUG("not currently implemented - ignored");
}

void otterPhaseBegin( const char* name )  {
    LOG_DEBUG("not currently implemented - ignored");
}

void otterPhaseEnd()  {
    LOG_DEBUG("not currently implemented - ignored");
}

void otterPhaseSwitch( const char* name )  {
    LOG_DEBUG("not currently implemented - ignored");
}
