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
// #include "otter/trace-archive.h"
#include "otter/otter-serial.h"
#include "otter/otter-structs.h"

static thread_data_t *thread_data = NULL;
// static task_data_t *initial_task_data = NULL;
// static task_data_t *implicit_task_data = NULL;

static otter_stack_t *regions = NULL;

void otterTraceBegin(void)
{
    // Initialise archive

    fprintf(stderr, "%s\n", __func__);

    /* detect environment variables */
    static otter_opt_t opt = {
        .hostname         = NULL,
        .tracename        = NULL,
        .tracepath        = NULL,
        .archive_name     = NULL,
        .append_hostname  = false
    };

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

    thread_data = new_thread_data(otter_thread_initial);
    trace_event_thread_begin(thread_data->location);

    // initial task
    task_data_t *initial_task_data = new_task_data(thread_data->location, NULL, 0, otter_task_initial, 0);
    stack_push(regions, (data_item_t) {.ptr = initial_task_data->region});
    trace_event_enter(thread_data->location, initial_task_data->region);

    return;
}

void otterTraceEnd(void)
{
    // Finalise arhchive
    fprintf(stderr, "%s\n", __func__);

    // initial task
    trace_event_leave(thread_data->location);

    /* For initial-task-end event, must manually record region defintion
        as it never gets handed off to an enclosing parallel region to be
        written at parallel-end */
    trace_region_def_t *initial_task_region = NULL;
    stack_pop(regions, (data_item_t*) &initial_task_region);
    trace_write_region_definition(initial_task_region);
    trace_destroy_task_region(initial_task_region);

    trace_event_thread_end(thread_data->location);
    thread_destroy(thread_data);
    trace_finalise_archive();
    return;
}

void otterParallelBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    /* assign space for this parallel region */
    parallel_data_t *parallel_data = new_parallel_data(thread_data->id,0L,NULL,0,0);
    trace_event_enter(thread_data->location, parallel_data->region);
    trace_region_def_t *initial_task_region = NULL;
    stack_peek(regions, (data_item_t*) &initial_task_region);
    task_data_t *implicit_task_data = new_task_data(thread_data->location, initial_task_region, 1, otter_task_implicit, 0);
    stack_push(regions, (data_item_t) {.ptr = implicit_task_data->region});
    trace_event_enter(thread_data->location, implicit_task_data->region);
    return;
}

void otterParallelEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    trace_event_leave(thread_data->location); // implicit task
    trace_event_leave(thread_data->location); // parallel
    return;
}

void otterTaskBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskBeginSingle(void)
{
    fprintf(stderr, "%s\n", __func__);
    trace_region_def_t *single = trace_new_workshare_region(thread_data->location, otter_work_single_executor, 1, 1);
    stack_push(regions, (data_item_t) {.ptr = single});
    trace_event_enter(
        thread_data->location,
        single
    );
    return;
}

void otterTaskEndSingle(void)
{
    fprintf(stderr, "%s\n", __func__);
    trace_region_def_t *single = NULL;
    stack_pop(regions, (data_item_t*) &single);
    trace_event_leave(thread_data->location);
    return;
}

void otterLoopBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopIterationBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopIterationEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterSynchroniseChildTasks(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterSynchroniseDescendantTasks(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}
