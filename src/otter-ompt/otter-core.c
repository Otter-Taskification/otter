#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#if !defined(__USE_POSIX)
#define __USE_POSIX // for HOST_NAME_MAX
#endif
#include <limits.h>
#include <unistd.h>         // gethostname
#include <sys/time.h>       // getrusage
#include <sys/resource.h>   // getrusage
#include <omp-tools.h>
#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-ompt.h"
#include "public/otter-trace/trace-thread-data.h"
#include "public/otter-trace/trace-task-data.h"
#include "public/otter-trace/trace-parallel-data.h"
#include "otter-ompt/callback.h"
#include "otter-ompt/otter.h"
#include "otter-ompt/otter-entry.h"

/* Static function prototypes */
static void print_resource_usage(void);

/* OMPT entrypoint signatures */
ompt_get_thread_data_t     get_thread_data;
ompt_get_parallel_info_t   get_parallel_info;

/* Register the tool's callbacks with otter-entry.c */
otter_opt_t *
tool_setup(
    tool_callbacks_t        *callbacks,
    ompt_function_lookup_t  lookup)
{
    include_callback(callbacks, ompt_callback_parallel_begin);
    include_callback(callbacks, ompt_callback_parallel_end);
    include_callback(callbacks, ompt_callback_thread_begin);
    include_callback(callbacks, ompt_callback_thread_end);
    include_callback(callbacks, ompt_callback_task_create);
    include_callback(callbacks, ompt_callback_task_schedule);
    include_callback(callbacks, ompt_callback_implicit_task);
    include_callback(callbacks, ompt_callback_work);
    include_callback(callbacks, ompt_callback_sync_region);
    #if defined(USE_OMPT_MASKED)
    include_callback(callbacks, ompt_callback_masked);
    #else
    include_callback(callbacks, ompt_callback_master);
    #endif

    get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
    get_parallel_info = 
        (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");

    static char host[HOST_NAME_MAX+1] = {0};
    gethostname(host, HOST_NAME_MAX);

    /* detect environment variables */
    static otter_opt_t opt = {
        .hostname         = NULL,
        .tracename        = NULL,
        .tracepath        = NULL,
        .archive_name     = NULL,
        .append_hostname  = false
    };

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

    return &opt;
}

void
tool_finalise(ompt_data_t *tool_data)
{
    trace_finalise_archive();
    print_resource_usage();

    otter_opt_t *opt = tool_data->ptr;

    char trace_folder[PATH_MAX] = {0};

    realpath(opt->tracepath, &trace_folder[0]);

    fprintf(stderr, "%s%s/%s\n",
        "OTTER_TRACE_FOLDER=", trace_folder, opt->archive_name);

    return;
}

static void
print_resource_usage(void)
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    #define PRINT_RUSAGE(key, val, units)\
        fprintf(stderr, "%35s: %8lu %s\n", key, usage.val, units);
    fprintf(stderr, "\nPROCESS RESOURCE USAGE:\n");
    PRINT_RUSAGE("maximum resident set size", ru_maxrss, "kb");
    PRINT_RUSAGE("page reclaims (soft page faults)", ru_minflt, "");
    PRINT_RUSAGE("page faults (hard page faults)", ru_majflt, "");
    PRINT_RUSAGE("block input operations", ru_inblock, "");
    PRINT_RUSAGE("block output operations", ru_oublock, "");
    #undef PRINT_RUSAGE

}

static void
on_ompt_callback_thread_begin(
    ompt_thread_t            thread_type,
    ompt_data_t             *thread)
{   
    thread_data_t *thread_data = new_thread_data(thread_type);
    thread->ptr = thread_data;

    LOG_DEBUG("[t=%lu] (event) thread-begin", thread_data->id);

    /* Record thread-begin event */
    trace_event_thread_begin(thread_data->location);

    return;
}

/* 
   The implicit parallel region does not dispatch a ompt_callback_parallel_end
   callback; however, the implicit parallel region can be finalized within this
   ompt_callback_thread_end callback. 
 */
static void
on_ompt_callback_thread_end(
    ompt_data_t             *thread)
{
    thread_data_t *thread_data = thread->ptr;

    LOG_DEBUG("[t=%lu] (event) thread-end", thread_data->id);
    LOG_DEBUG_IF((thread_data->type == ompt_thread_initial),
        "final clean-up...");

    /* Record thread-end event */
    trace_event_thread_end(thread_data->location);

    /* Destroy thread data (also destroys thread_data->location) */
    thread_destroy(thread_data);

    return;
}

static void
on_ompt_callback_parallel_begin(
    ompt_data_t             *encountering_task,
    const ompt_frame_t      *encountering_task_frame,
    ompt_data_t             *parallel,
    unsigned int             requested_parallelism,
    int                      flags,
    const void              *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;
    task_data_t *task_data = (task_data_t*) encountering_task->ptr;

    LOG_DEBUG("[t=%lu] (event) parallel-begin", thread_data->id);

    thread_data->is_master_thread = true;

    /* assign space for this parallel region */
    parallel_data_t *parallel_data = new_parallel_data(
        thread_data->id,
        // task_data ? trace_task_get_id(task_data) : OTF2_UNDEFINED_UINT64,
        trace_task_get_id(task_data),
        task_data,
        requested_parallelism,
        flags);
    parallel->ptr = parallel_data;

    /* record enter region event */
    trace_event_enter(thread_data->location, trace_parallel_get_region_def(parallel_data));

    return;
}

static void
on_ompt_callback_parallel_end(
    ompt_data_t *parallel,
    ompt_data_t *encountering_task,
    int          flags,
    const void  *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;

    LOG_DEBUG("[t=%lu] (event) parallel-end", thread_data->id);

    if ((parallel == NULL) || (parallel->ptr == NULL))
    {
        LOG_ERROR("parallel end: null pointer");
    } else {
        parallel_data_t *parallel_data = parallel->ptr;
        trace_event_leave(thread_data->location);
        /* reset flag */
        thread_data->is_master_thread = false;
    }

    return;
}


/* p 467

   Used for callbacks that are dispatched when taskregions or initial tasks are
   generated

   encountering_task, encountering_task_frame are NULL for an initial task

   flags:

        typedef enum ompt_task_flag_t {
            ompt_task_initial    = 0x00000001,
            ompt_task_implicit   = 0x00000002,
            ompt_task_explicit   = 0x00000004,
            ompt_task_target     = 0x00000008,
            ompt_task_undeferred = 0x08000000,
            ompt_task_untied     = 0x10000000,
            ompt_task_final      = 0x20000000,
            ompt_task_mergeable  = 0x40000000,
            ompt_task_merged     = 0x80000000
        } ompt_task_flag_t;

   explicit task: any task that is not an implicit task

   implicit task: a task generated by an implicit parallel region or when a 
    parallel construct is encountered

   initial task: a type of implicit task associated with an implicit parallel
    region

   Events:
    task-create

   task generating constructs:
    task (create a task from a region)
    taskloop (create tasks from loop iterations)
    target (generates a target task)
    target update (generates a target task)
    target enter/exit data (generates a target task)
 */
static void
on_ompt_callback_task_create(
    ompt_data_t         *encountering_task,
    const ompt_frame_t  *encountering_task_frame,
    ompt_data_t         *new_task,
    int                  flags,
    int                  has_dependences,
    const void          *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;
    LOG_DEBUG("[t=%lu] BEGIN EVENT", thread_data->id);

    /* Intel runtime seems to give the initial task a task-create event while
       LLVM just gives it an implicit-task-begin event. If invoked by Intel,
       defer initial task data creation until the implicit-task-begin event for
       simplicity */
    if (flags & ompt_task_initial)
    {
        LOG_DEBUG("ignored intial-task-create event");
        return;
    }

    LOG_DEBUG("[t=%lu] (event) task-create", thread_data->id);
    LOG_DEBUG("[t=%lu] codeptr_ra %p", thread_data->id, codeptr_ra);

    /* get the task data of the parent, if it exists */
    task_data_t *parent_task_data = NULL;
    if (flags & ompt_task_initial) {
        parent_task_data = (task_data_t*) encountering_task->ptr;
    }

    trace_region_def_t *parent_task_region = NULL;
    if (parent_task_data != NULL) {
        parent_task_region = trace_task_get_region_def(parent_task_data);
    }

    /* make space for the newly-created task */
    task_data_t *task_data = new_task_data(thread_data->location, parent_task_region, flags, has_dependences, NULL, codeptr_ra);

    trace_region_def_t *task_region = trace_task_get_region_def(task_data);
    trace_location_store_region_def(thread_data->location, task_region);
    trace_event_task_create(thread_data->location, task_region);

    new_task->ptr = task_data;

    LOG_DEBUG_TASK_TYPE(thread_data->id, 
        parent_task_data ? trace_task_get_id(parent_task_data) : 0L, trace_task_get_id(task_data), flags);
    
    LOG_DEBUG("[t=%lu] END EVENT", thread_data->id);
    return;
}

static void
on_ompt_callback_task_schedule(
    ompt_data_t             *prior_task,
    ompt_task_status_t       prior_task_status,
    ompt_data_t             *next_task)
{

    LOG_DEBUG_PRIOR_TASK_STATUS(prior_task_status);

    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;

    if (prior_task_status == ompt_task_early_fulfill 
        || prior_task_status == ompt_task_late_fulfill)
    {
        LOG_INFO("ignored task-fulfill event");
        return;
    }

    task_data_t *prior_task_data = NULL, *next_task_data = NULL;

    /* next_task is NULL for a task-fulfill event */

    prior_task_data = (task_data_t*) prior_task->ptr;
    next_task_data  = (task_data_t*) next_task->ptr;

    LOG_DEBUG("[t=%lu] (event) task-schedule %lu (%d) -> %lu",
        thread_data->id,
        trace_task_get_id(prior_task_data),
        prior_task_status,
        trace_task_get_id(next_task_data)
    );

#if defined(TASK_SCHEDULE_LEAVE_ENTER)
    // Deprecated
    if (prior_task_data->type == ompt_task_explicit 
        || prior_task_data->type == ompt_task_target)
    {
        trace_event_task_schedule(thread_data->location,
            trace_task_get_region_def(prior_task_data), prior_task_status);
        trace_event_leave(thread_data->location);
    }

    if (next_task_data->type == ompt_task_explicit 
        || next_task_data->type == ompt_task_target)
    {
        /* reset status on task-entry */
        trace_event_task_schedule(thread_data->location,
            trace_task_get_region_def(prior_task_data), 0); /* no status */
        trace_event_enter(thread_data->location, trace_task_get_region_def(next_task_data));
    }
#else
    // Default is to record task-switch event
    trace_event_task_switch(
        thread_data->location,
        trace_task_get_region_def(prior_task_data),
        prior_task_status,
        trace_task_get_region_def(next_task_data)
    );
#endif
    
    return;
}

static void
on_ompt_callback_implicit_task(
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    unsigned int             actual_parallelism,
    unsigned int             index,
    int                      flags)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;

    /* Only handle implicit-task events */
    // if (!(flags & ompt_task_implicit)) return;

    LOG_DEBUG("[t=%lu] (event) %s-task-%s",
        thread_data->id,
        flags & ompt_task_initial ? "initial" :
            flags & ompt_task_implicit ? "implicit" : "???",
        endpoint == ompt_scope_begin ? "begin" : "end");

    if (endpoint == ompt_scope_begin)
    {
        /* For initial-task-begin event:
            - LLVM:  parallel != NULL
            - Intel: parallel == NULL
         */
        parallel_data_t *parallel_data = NULL;
        if (flags & ompt_task_implicit)
            parallel_data = (parallel_data_t*) parallel->ptr;

        /* Worker threads record parallel-begin during implicit-task-begin */
        if (index != 0 && (flags & ompt_task_implicit))
            trace_event_enter(thread_data->location, trace_parallel_get_region_def(parallel_data));

        /* Create implicit task data __after__ parallel-begin so that the OTF2
           region is added to the queue for the new parallel region */
        task_data_t *encountering_task_data = NULL;
        trace_region_def_t *encountering_task_region = NULL;
        if (flags & ompt_task_implicit) {
            encountering_task_data = trace_parallel_get_task_data(parallel_data);
            encountering_task_region = trace_task_get_region_def(encountering_task_data);
        }
        task_data_t *implicit_task_data = new_task_data(
            thread_data->location,
            encountering_task_region,
            flags,
            0,
            NULL,
            NULL
        );
        task->ptr = implicit_task_data;

        trace_region_def_t *implicit_task_region = trace_task_get_region_def(implicit_task_data);
        trace_location_store_region_def(thread_data->location, implicit_task_region);
        /* Enter implicit task region */
        trace_event_enter(thread_data->location, implicit_task_region);

    } else {

        task_data_t *implicit_task_data = (task_data_t*)task->ptr;

        /* Update implicit task status */
        trace_event_task_schedule(thread_data->location,
            trace_task_get_region_def(implicit_task_data), ompt_task_complete);

        /* Leave implicit task region */
        trace_event_leave(thread_data->location);

        /* Worker threads record parallel-end during implicit-task-end
            callback */
        if (index != 0 && (flags & ompt_task_implicit))
            trace_event_leave(thread_data->location);

        /* For initial-task-end event, must manually record region defintion
           as it never gets handed off to an enclosing parallel region to be
           written at parallel-end */
        if (flags & ompt_task_initial)
        {
            trace_write_region_definition(trace_task_get_region_def(implicit_task_data));
            trace_destroy_task_region(trace_task_get_region_def(implicit_task_data));
        }
    }
    return;
}

/* Used for callbacks that are dispatched when worksharing regions, loop-related
   regions, and taskloopregions begin and end.

   Events:
    section-begin/end (scope=implicit task)
    single-begin/end
    workshare-begin/end (scope=implicit task)
    ws-loop-begin/end (scope=implicit task)
    distribute-begin/end (scope=implicit task)
    taskloop-begin/end (scope=encountering task)

 */
static void
on_ompt_callback_work(
    ompt_work_t              wstype,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    uint64_t                 count,
    const void              *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;
    task_data_t *task_data = (task_data_t*) task->ptr;

    LOG_DEBUG_WORK_TYPE(thread_data->id, wstype, count,
        endpoint==ompt_scope_begin?"begin":"end");

    if (wstype != ompt_work_workshare && wstype != ompt_work_distribute)
    {
        if (endpoint == ompt_scope_begin)
        {
            trace_region_def_t *wshare_rgn = trace_new_workshare_region(
                /* Convert the OMPT enum type to a generic Otter enum type */
                wstype == ompt_work_loop ? otter_work_loop :
                    wstype == ompt_work_single_executor ? otter_work_single_executor :
                    wstype == ompt_work_single_other ? otter_work_single_other :
                    wstype == ompt_work_taskloop ? otter_work_taskloop : 0,
                count,
                trace_task_get_id(task_data)
            );
            trace_location_store_region_def(thread_data->location, wshare_rgn);
            trace_event_enter(thread_data->location, wshare_rgn);
        } else {
            trace_event_leave(thread_data->location);
        }
    }

    return;
}

/* Used for callbacks that are dispatched when master regions start and end.

    NOTE: deprecated in 5.1 and replaced with ompt_callback_masked
 */
static void
#if defined(USE_OMPT_MASKED)
on_ompt_callback_masked(
#else
on_ompt_callback_master(
#endif
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;
    trace_location_def_t *location = thread_data->location;
    task_data_t *task_data = (task_data_t*) task->ptr;
    unique_id_t location_id = trace_location_get_id(location);
    unique_id_t task_id = trace_task_get_id(task_data);

    LOG_DEBUG("[t=%lu] (event) master-%s", 
        thread_data->id, endpoint==ompt_scope_begin?"begin":"end");

    if (endpoint == ompt_scope_begin)
    {
        trace_region_def_t *master_rgn = trace_new_master_region(location_id, task_id);
        trace_location_store_region_def(location, master_rgn);
        trace_event_enter(location, master_rgn);
    } else {
        trace_event_leave(location);
    }
    return;
}

static void
on_ompt_callback_sync_region(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;
    task_data_t *task_data = (task_data_t*) task->ptr;

    LOG_DEBUG("[t=%lu] (event) sync-region-%s (%s)",
        thread_data->id, endpoint == ompt_scope_begin ? "begin" : "end",
        kind == ompt_sync_region_barrier
                ? "barrier" :
        kind == ompt_sync_region_barrier_implicit
                ? "implicit barrier" :
        kind == ompt_sync_region_barrier_explicit
                ? "explicit barrier" :
        kind == ompt_sync_region_barrier_implementation
                ? "implementation barrier" :
        kind == ompt_sync_region_taskwait
                ? "taskwait" :
        kind == ompt_sync_region_taskgroup
                ? "taskgroup" :
        kind == ompt_sync_region_reduction
                ? "reduction" : "unknown"
    );

    if (endpoint == ompt_scope_begin)
    {
        trace_region_def_t *sync_rgn = trace_new_sync_region(
            kind,
            kind == ompt_sync_region_taskgroup ? trace_sync_descendants : trace_sync_children,
            trace_task_get_id(task_data)
        );
        trace_location_store_region_def(thread_data->location, sync_rgn);
        trace_event_enter(thread_data->location, sync_rgn);
    } else {
        trace_event_leave(thread_data->location);
    }
    return;
}
