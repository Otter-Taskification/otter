#define __USE_POSIX // HOST_NAME_MAX
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <execinfo.h>

#include "public/otter-version.h"
#include "public/debug.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-ompt.h"
#include "api/otter-serial/otter-serial.h"

#include "public/otter-trace/trace-thread-data.h"
#include "public/otter-trace/trace-task-data.h"
#include "public/otter-trace/trace-parallel-data.h"

#define LOG_EVENT_CALL(file, func, line, ifunc) LOG_DEBUG("%s:%d in %s", file, line, func)

static thread_data_t *thread_data = NULL;
static trace_location_def_t *location = NULL;
static unique_id_t thread_id = 0;
static otter_stack_t *task_stack = NULL;
static otter_stack_t *parallel_stack = NULL;
static bool tracingActive = false;

/* detect environment variables */
static otter_opt_t opt = {
    .hostname         = NULL,
    .tracename        = NULL,
    .tracepath        = NULL,
    .archive_name     = NULL,
    .append_hostname  = false
};

static task_data_t *get_encountering_task(void)
{
    task_data_t *t = NULL;
    stack_peek(task_stack, (data_item_t*) &t);
    assert(t != NULL);
    return t;
}

void otterTraceInitialise(const char* file, const char* func, const int line)
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

    task_stack = stack_create();
    parallel_stack = stack_create();

    tracingActive = true;

    thread_data = new_thread_data(otter_thread_initial);
    location = thread_data->location;
    thread_id = thread_data->id;
    trace_event_thread_begin(location);

    otter_src_location_t src_location = {
        .file = file,
        .func = func,
        .line = line
    };
    LOG_EVENT_CALL(src_location.file, src_location.func, src_location.line, __func__);

    // initial task
    task_data_t *initial_task = new_task_data(
        location,
        NULL,
        otter_task_initial,
        0,
        &src_location,
        NULL
    );

    trace_region_def_t *initial_task_region = trace_task_get_region_def(initial_task);
    trace_location_store_region_def(location, initial_task_region);
    stack_push(task_stack, (data_item_t) {.ptr = initial_task});
    trace_event_enter(location, initial_task_region);
    return;
}

void otterTraceFinalise(void)
{
    // Finalise arhchive
    LOG_DEBUG("=== finalising archive ===");

    // initial task
    trace_event_leave(location);

    /* For initial-task-end event, must manually record region defintion
        as it never gets handed off to an enclosing parallel region to be
        written at parallel-end */
    trace_region_def_t *initial_task_region = NULL;
    trace_location_get_region_def(location, &initial_task_region);
    LOG_DEBUG("writing initial-task region definition from thread queue: %p", initial_task_region);
    trace_write_region_definition(initial_task_region);
    trace_destroy_task_region(initial_task_region);
    initial_task_region = NULL;

    task_data_t *initial_task = NULL;
    stack_pop(task_stack, (data_item_t*) &initial_task);
    assert(trace_task_get_flags(initial_task) == otter_task_initial);
    task_destroy(initial_task);
    initial_task = NULL;

    /*
        If there are any region definitions left in the queue, this means
        they have not been written to the trace. This can happen with the
        otter-serial event source e.g. if a phase region is inserted outside
        and parallel region.
    */
    size_t num_definitions = trace_location_get_num_region_def(location);
    LOG_DEBUG_IF((num_definitions!=0), "definitions in thread queue: %lu", num_definitions);
    trace_region_def_t *region = NULL;
    while (trace_location_get_region_def(location, &region)) {
        trace_region_type_t region_type = trace_region_get_type(region);
        LOG_DEBUG("writing region definition %p (%d)", region, region_type);
        trace_write_region_definition(region);
        switch (region_type) {
            /*
            Only phase regions may appear outside a parallel region.
            */
            case trace_region_phase:
                trace_destroy_phase_region(region);
                break;
            default:
                LOG_ERROR("unexpected region type %d", region_type);
                abort();
        }
    }

    trace_event_thread_end(location);
    thread_destroy(thread_data);
    trace_finalise_archive();

    stack_destroy(task_stack, false, NULL);
    stack_destroy(parallel_stack, false, NULL);

    char trace_folder[PATH_MAX] = {0};

    realpath(opt.tracepath, &trace_folder[0]);

    fprintf(stderr, "%s%s/%s\n",
        "OTTER_TRACE_FOLDER=", trace_folder, opt.archive_name);

    return;
}

void otterThreadsBegin(const char* file, const char* func, const int line)
{
    if (!tracingActive)
    {
        LOG_WARN("recording mandatory event while tracing stopped");
    }    

    task_data_t *encountering_task = get_encountering_task();

    parallel_data_t *parallel_data = new_parallel_data(
        thread_id,
        trace_task_get_id(encountering_task),
        encountering_task,
        0,
        0
    );

    trace_region_def_t *parallel_region = trace_parallel_get_region_def(parallel_data);

    stack_push(parallel_stack, (data_item_t) {.ptr = parallel_data});

    trace_event_enter(location, parallel_region);

    otter_src_location_t src_location = {
        .file = file,
        .func = func,
        .line = line
    };
    LOG_EVENT_CALL(src_location.file, src_location.func, src_location.line, __func__);

    task_data_t *implicit_task = new_task_data(
        location,
        trace_task_get_region_def(encountering_task),
        otter_task_implicit,
        0,
        &src_location,
        NULL
    );

    trace_region_def_t *implicit_task_region = trace_task_get_region_def(implicit_task);
    trace_location_store_region_def(location, implicit_task_region);

    stack_push(task_stack, (data_item_t) {.ptr = implicit_task});

    trace_event_enter(location, implicit_task_region);

    return;
}

void otterThreadsEnd(void)
{
    if (!tracingActive)
    {
        LOG_WARN("recording mandatory event while tracing stopped");
    }   

    task_data_t *implicit_task = NULL;
    parallel_data_t *parallel_data = NULL;

    stack_pop(task_stack, (data_item_t*) &implicit_task);
    trace_event_leave(location); // implicit task
    task_destroy(implicit_task);

    stack_pop(parallel_stack, (data_item_t*) &parallel_data);
    trace_event_leave(location); // parallel
    parallel_destroy(parallel_data);
    return;
}

void otterTaskBegin(const char* file, const char* func, const int line)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    otter_src_location_t src_location = {
        .file = file,
        .func = func,
        .line = line
    };
    LOG_EVENT_CALL(src_location.file, src_location.func, src_location.line, __func__);

    void *return_address[2] = {0};
    // backtrace() adds a stackframe, so want the one added due to the 
    // otterTaskBegin call()
    backtrace((void**)&return_address, 2);
    LOG_DEBUG("return_address %p", return_address[1]);

    task_data_t *encountering_task = get_encountering_task();
    trace_region_def_t *encountering_task_region = trace_task_get_region_def(encountering_task);

    task_data_t *task = new_task_data(
        location,
        encountering_task_region,
        otter_task_explicit,
        0,
        &src_location,
        return_address[1]
    );

    trace_region_def_t *task_region = trace_task_get_region_def(task);
    trace_location_store_region_def(location, task_region);

    stack_push(task_stack, (data_item_t) {.ptr = task});

    trace_event_task_create(location, task_region);

    trace_event_task_switch(
        location,
        encountering_task_region,
        otter_task_switch,
        task_region
    );

    return;
}

void otterTaskEnd(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    LOG_DEBUG();

    task_data_t *task = NULL;
    trace_region_def_t *task_region = NULL;

    stack_pop(task_stack, (data_item_t*) &task);

    task_data_t *encountering_task = get_encountering_task();

    trace_event_task_switch(
        location,
        trace_task_get_region_def(task),
        otter_task_complete,
        trace_task_get_region_def(encountering_task)
    );

    task_destroy(task);

    return;
}

void otterLoopBegin()
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    task_data_t *encountering_task = get_encountering_task();

    trace_region_def_t *loop = trace_new_workshare_region(
        otter_work_loop,
        1,
        trace_task_get_id(encountering_task)
    );


    trace_location_store_region_def(location, loop);
    trace_event_enter(location, loop);

    return;
}

void otterLoopEnd(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }
    trace_event_leave(location);
    return;
}

void otterLoopIterationBegin()
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    // LOG_EVENT_CALL(file, func, line, __func__);
    return;
}

void otterLoopIterationEnd(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    
    return;
}

void otterSynchroniseTasks(otter_task_sync_t mode)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    // LOG_EVENT_CALL(file, func, line, __func__);
    task_data_t *encountering_task = get_encountering_task();
    trace_region_def_t *taskwait = trace_new_sync_region(
        otter_sync_region_taskwait,
        mode == otter_sync_descendants ? trace_sync_descendants : trace_sync_children,
        trace_task_get_id(encountering_task)
    );
    trace_location_store_region_def(location, taskwait);
    trace_event_enter(location, taskwait);
    trace_event_leave(location);
    return;
}

void otterSynchroniseDescendantTasksBegin()
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    // LOG_EVENT_CALL(file, func, line, __func__);
    task_data_t *encountering_task = get_encountering_task();
    trace_region_def_t *taskgroup = trace_new_sync_region(
        otter_sync_region_taskgroup,
        trace_sync_descendants,
        trace_task_get_id(encountering_task)
    );
    trace_location_store_region_def(location, taskgroup);
    trace_event_enter(location, taskgroup);
    return;
}

void otterSynchroniseDescendantTasksEnd(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }
    trace_event_leave(location);
    return;
}

void otterTraceStart(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("tracing interface started");
        tracingActive = true;
    } else {
        LOG_DEBUG("tracing interface already active");
    }
    return;
}

void otterTraceStop(void)
{
    if (tracingActive)
    {
        LOG_DEBUG("tracing interface stopped");
        tracingActive = false;
    } else {
        LOG_DEBUG("tracing interface already inactive");
    }
    return;
}


void otterPhaseBegin(const char* name)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    LOG_DEBUG("start phase: \"%s\"", name);

    task_data_t *encountering_task = get_encountering_task();
    trace_region_def_t *phase = trace_new_phase_region(
        otter_phase_region_generic,
        trace_task_get_id(encountering_task),
        name
    );
    trace_location_store_region_def(location, phase);
    trace_event_enter(location, phase);
    return;
}


void otterPhaseSwitch(const char* name)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    LOG_DEBUG("switching to phase: \"%s\"", name);

    otterPhaseEnd();
    otterPhaseBegin(name);
}


void otterPhaseEnd(void)
{
    if (!tracingActive)
    {
        LOG_DEBUG("[INACTIVE]");
        return;
    }

    LOG_DEBUG("end phase");
    trace_event_leave(location);
    return;
}
