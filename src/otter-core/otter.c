#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>         // gethostname
#include <sys/time.h>       // getrusage
#include <sys/resource.h>   // getrusage

#include <otf2/otf2.h>

#include <macros/debug.h>
#include <macros/callback.h>

#include <otter-ompt-header.h>

#include <otter-common.h>
#include <otter-core/otter.h>
#include <otter-core/otter-structs.h>
#include <otter-core/otter-entry.h>
#include <otter-core/otter-environment-variables.h>
#include <otter-trace/trace.h>
#include <otter-trace/trace-structs.h>

/* Static function prototypes */
static void print_resource_usage(void);

/* OMPT entrypoint signatures */
ompt_get_thread_data_t     get_thread_data;
ompt_get_parallel_info_t   get_parallel_info;

/* Register the tool's callbacks with otter-entry.c */
void
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

    return;
}

void
tool_finalise(void)
{
    trace_finalise_archive();
    print_resource_usage();
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

    fprintf(stderr, "\n%35s: %8lu %s\n", "threads",
        get_unique_thread_id(), "");
    fprintf(stderr, "%35s: %8lu %s\n", "parallel regions",
        get_unique_parallel_id(), "");
    fprintf(stderr, "%35s: %8lu %s\n", "tasks",
        get_unique_task_id()-1, "");
}

static void
on_ompt_callback_thread_begin(
    ompt_thread_t            thread_type,
    ompt_data_t             *thread)
{   
    thread_data_t *thread_data = new_thread_data(thread_type);
    thread->ptr = thread_data;

    LOG_DEBUG_THREAD_TYPE(thread_type, thread_data->id);

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

    LOG_DEBUG_THREAD_TYPE(thread_data->type, thread_data->id);
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

    LOG_DEBUG("[t=%lu] %-6s %s", thread_data->id, "begin", "parallel");

    thread_data->is_master_thread = true;

    /* assign space for this parallel region */
    parallel_data_t *parallel_data = new_parallel_data(thread_data->id,
        requested_parallelism, flags);
    parallel->ptr = parallel_data;

    /* record enter region event */
    trace_event_enter(thread_data->location, parallel_data->region);

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

    if ((parallel == NULL) || (parallel->ptr == NULL))
    {
        LOG_ERROR("parallel end: null pointer");
    } else {
        LOG_DEBUG("[t=%lu] %-6s %s", thread_data->id, "end", "parallel");
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

    /* Intel runtime seems to give the initial task a task-create event while
       LLVM just gives it an implicit-task-begin event. If compiling with Intel,
       defer initial task data creation until the implicit-task-begin event for
       simplicity */
    #if defined(__INTEL_COMPILER)
    if (flags & ompt_task_initial)
    {
        LOG_DEBUG("Intel detected: defer intial-task-begin event until "
                  "implicit-task-begin");
        return;
    }
    #endif

    /* get the task data of the parent, if it exists */
    task_data_t *parent_task_data = flags & ompt_task_initial ? 
        NULL : (task_data_t*) encountering_task->ptr;

    /* make space for the newly-created task */
    task_data_t *task_data = new_task_data(thread_data->location, 
        parent_task_data ? parent_task_data->region : NULL, 
        get_unique_task_id(), flags, has_dependences);

    /* record the task-create event */
    trace_event_task_create(thread_data->location, task_data->region);

    new_task->ptr = task_data;

    LOG_DEBUG_TASK_TYPE(thread_data->id, 
        parent_task_data ? 0L : parent_task_data->id, task_data->id, flags);
    
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

    if (prior_task_data->type == ompt_task_explicit 
        || prior_task_data->type == ompt_task_target)
    {
        trace_event_task_schedule(thread_data->location,
            prior_task_data->region, prior_task_status);
        trace_event_leave(thread_data->location);
    }

    if (next_task_data->type == ompt_task_explicit 
        || next_task_data->type == ompt_task_target)
    {
        /* reset status on task-entry */
        trace_event_task_schedule(thread_data->location,
            prior_task_data->region, 0); /* no status */
        trace_event_enter(thread_data->location, next_task_data->region);
    }
    
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
    if (!(flags & ompt_task_implicit)) return;

    /* Get the enclosing parallel region data - parallel arg is NULL for
       implicit-task-end event */
    if (get_parallel_info(0, &parallel, NULL) != 2)
    {
        LOG_ERROR("parallel data unavailable");
        abort();
    }
    parallel_data_t *parallel_data = (parallel_data_t*) parallel->ptr;
    

    if (endpoint == ompt_scope_begin)
    {
        task_data_t *task_data = new_task_data(
            thread_data->location,
            NULL,
            get_unique_task_id(),
            flags,
            0);
        task->ptr = task_data;

		LOG_DEBUG_IMPLICIT_TASK(flags, "begin", task_data->id);

        /* Worker threads record parallel-begin during implicit-task-begin */
        // if (!thread_data->is_master_thread)
        if (thread_data->id != parallel_data->master_thread)
            trace_event_enter(thread_data->location, parallel_data->region);

    } else {

        LOG_DEBUG_IMPLICIT_TASK(flags, "end", ~(0L));

        /* Worker threads record parallel-end during implicit-task-end
            callback */
        // if (!thread_data->is_master_thread)
        if (thread_data->id != parallel_data->master_thread)
            trace_event_leave(thread_data->location);
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
                thread_data->location, wstype, count);
            trace_event_enter(thread_data->location, wshare_rgn);
        } else {
            trace_event_leave(thread_data->location);
        }
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

    LOG_DEBUG("[t=%lu] %-6s %s (task=%lu, type=%d)",
        thread_data->id,
        endpoint == ompt_scope_begin ? "begin" : "end",
        kind == ompt_sync_region_barrier                ? "barrier" :
        kind == ompt_sync_region_barrier_implicit       ? "barrier_implicit" :
        kind == ompt_sync_region_barrier_explicit       ? "barrier_explicit" :
        kind == ompt_sync_region_barrier_implementation ? "barrier_implementation" :
        kind == ompt_sync_region_taskwait               ? "taskwait" :
        kind == ompt_sync_region_taskgroup              ? "taskgroup" :
        kind == ompt_sync_region_reduction              ? "reduction" : "unknown",
        task_data->id,
        task_data->type
    );

    if (endpoint == ompt_scope_begin)
    {
        trace_region_def_t *sync_rgn = trace_new_sync_region(
            thread_data->location, kind, task_data->id);
        trace_event_enter(thread_data->location, sync_rgn);
    } else {
        trace_event_leave(thread_data->location);
    }
    return;
}

unique_id_t
get_unique_id(unique_id_type_t id_type)
{
    /* start counting tasks from 1 so that the initial task is always #1, and
       the root node of the task graph will have ID 0 not attached to any real
       task

       count parallel regions from 1 so the implicit parallel region around the
       whole program is always 0
     */
    static unique_id_t id[NUM_ID_TYPES] = {0,1,0,1};
    return __sync_fetch_and_add(&id[id_type], 1L);
}
