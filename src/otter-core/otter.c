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
#include <otter-core/otter-entry.h>
#include <otter-core/otter.h>
#include <otter-datatypes/graph.h>
#include <otter-task-graph/task-graph.h>
#include <otter-trace/trace.h>

/* Static function prototypes */
static void print_resource_usage(void);
static unique_id_t get_unique_id(unique_id_type_t id_type);

/* Callback used by task-graph to allow otter to control destruction of graph
   node data */
static void destroy_graph_node_data(
    void *node_data, graph_node_type_t node_type);

/* Task data constructor */
static task_data_t *new_task_data(
    unique_id_t      id,
    ompt_task_flag_t flags,
    unique_id_t      parallel
);

/* OMPT entrypoint signatures */
ompt_get_thread_data_t     get_thread_data;
ompt_get_parallel_info_t   get_parallel_info;

/* Register the tool's callbacks with otter-entry which will pass them on to OMP
*/
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
    // include_callback(callbacks, ompt_callback_task_schedule);
    include_callback(callbacks, ompt_callback_implicit_task);
    // include_callback(callbacks, ompt_callback_work);

    get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
    get_parallel_info = 
        (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");

    static char host[HOST_NAME_MAX+1] = {0};
    gethostname(host, HOST_NAME_MAX);

    /* detect environment variables for graph output file */
    static otter_opt_t opt = {
        .hostname         = NULL,
        .graph_output     = NULL,
        .graph_format     = NULL,
        .graph_nodeattr   = NULL,
        .append_hostname  = false
    };

    opt.hostname = host;
    opt.graph_output = getenv("OTTER_TASK_GRAPH_OUTPUT");
    opt.graph_format = getenv("OTTER_TASK_GRAPH_FORMAT");
    opt.graph_nodeattr = getenv("OTTER_TASK_GRAPH_NODEATTR");
    opt.append_hostname = 
        getenv("OTTER_APPEND_HOSTNAME") == NULL ? false : true;

    LOG_INFO("Otter environment variables:");
    LOG_INFO("%-30s %s", "host", opt.hostname);
    LOG_INFO("%-30s %s", "OTTER_TASK_GRAPH_OUTPUT", opt.graph_output);
    LOG_INFO("%-30s %s", "OTTER_TASK_GRAPH_FORMAT", opt.graph_format);
    LOG_INFO("%-30s %s", "OTTER_TASK_GRAPH_NODEATTR", opt.graph_nodeattr);
    LOG_INFO("%-30s %s",
        "OTTER_APPEND_HOSTNAME",opt.append_hostname ? "Yes" : "No");

    task_graph_init(&opt);
    trace_initialise_archive(&opt);

    return;
}

static void
destroy_graph_node_data(
    void               *node_data,
    graph_node_type_t   node_type)
{
    LOG_DEBUG("(0x%x) %p", node_type, node_data);
    if (FLAG_NODE_TYPE_END(node_type)) free(node_data); // prevent double-free
    return;
}

void
tool_finalise(void)
{
    task_graph_write();
    task_graph_destroy(&destroy_graph_node_data);
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

/* 

   Events:
   initial-thread-begin

 */
static void
on_ompt_callback_thread_begin(
    ompt_thread_t            thread_type,
    ompt_data_t             *thread)
{   
    thread_data_t *thread_data = malloc(sizeof(*thread_data));
    *thread_data = (thread_data_t) {
        .id = get_unique_thread_id(),
        .location = NULL,
        .region_context_stack = stack_create(NULL)
    };
    thread->ptr = thread_data;

    thread_data->location = trace_new_location_definition(
        thread_data->id,
        OTF2_LOCATION_TYPE_CPU_THREAD,
        DEFAULT_LOCATION_GRP);
    trace_event_thread_begin(thread_data->location);

    LOG_DEBUG_THREAD_TYPE(thread_type, thread_data->id);
    return;
}

/* 
   
   Events:
   initial-thread-end

   A thread dispatches a registered ompt_callback_thread_end callback for the
   initial-thread-end event in that thread. The callback occurs in the context
   of the thread. The callback has type signature ompt_callback_thread_end_t.
   The implicit parallel region does not dispatch a ompt_callback_parallel_end
   callback; however, the implicit parallel region can be finalized within this
   ompt_callback_thread_end callback. 

 */
static void
on_ompt_callback_thread_end(
    ompt_data_t             *thread)
{
    if ((thread != NULL) && (thread->ptr != NULL)) 
    {
        thread_data_t *thread_data = thread->ptr;
        LOG_DEBUG_THREAD_TYPE(ompt_thread_unknown, thread_data->id);
        trace_event_thread_end(thread_data->location);
        stack_destroy(thread_data->region_context_stack, false);
        free (thread_data);
    }
    return;
}

/* 
   
   implicit parallel region: a parallel region, executed by one thread, not 
    generated by a parallel construct. They surround the whole OpenMP program,
    all target regions and all teams regions.

   Events:
   parallel-begin

 */
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

    /* assign space for this parallel region */
    parallel_data_t *parallel_data = malloc(sizeof(*parallel_data));
    *parallel_data = (parallel_data_t) {
        .id = get_unique_parallel_id(),
        .parallel_begin_node_ref = NULL,
        .parallel_end_node_ref = NULL,
        .region = NULL
    };
    parallel->ptr = parallel_data;

    /* Create a new region context */
    region_context_t *parallel_context = malloc(sizeof(*parallel_context));
    *parallel_context = (region_context_t) {
        .type = context_parallel,
        .context_data = parallel_data,
        .context_task_graph_nodes = stack_create(NULL),
        .context_begin_node = NULL,
        .context_end_node = NULL,
        .lock = PTHREAD_MUTEX_INITIALIZER
    };
    parallel_data->context = parallel_context;

    /* create trace region definition */
    parallel_data->region = trace_new_region_definition(
        parallel_data->id, OTF2_REGION_ROLE_PARALLEL);

    /* record enter region event */
    trace_event_enter(thread_data->location, parallel_data->region);

    /* add node representing the start of a parallel region to the graph */
    parallel_context->context_begin_node = task_graph_add_node(
        node_context_parallel_begin,
        (task_graph_node_data_t) {.ptr = parallel_data}
    );
    parallel_data->parallel_begin_node_ref = 
        parallel_context->context_begin_node;

    /* declare an edge from the encountering task to the parallel-begin node */
    task_graph_add_edge(
        thread_data->initial_task_graph_node_ref,
        parallel_context->context_begin_node);

    LOG_DEBUG_PARALLEL_RGN_TYPE(flags, parallel_data->id);

    return;
}

/* 

   Events:
   parallel-end

 */
static void
on_ompt_callback_parallel_end(
    ompt_data_t *parallel,
    ompt_data_t *encountering_task,
    int          flags,
    const void  *codeptr_ra)
{
    thread_data_t *thread_data = (thread_data_t*) get_thread_data()->ptr;

    region_context_t *context = NULL;

    if ((parallel == NULL) || (parallel->ptr == NULL))
    {
        LOG_ERROR("parallel end: null pointer");
    } else {
        parallel_data_t *parallel_data = parallel->ptr;
        context = parallel_data->context;
        LOG_DEBUG_PARALLEL_RGN_TYPE(flags, parallel_data->id);

        trace_event_leave(thread_data->location, parallel_data->region);

        /* add parallel-end node */
        context->context_end_node = task_graph_add_node(
            node_context_parallel_end,
            (task_graph_node_data_t) {.ptr = parallel_data}
        );
        parallel_data->parallel_end_node_ref = context->context_end_node;

        /* make sure nodes connect to context-end node */
        if (stack_size(context->context_task_graph_nodes) == 0)
        {
            task_graph_add_edge(context->context_begin_node,
                context->context_end_node);
        } else {
            task_graph_node_t *graph_node = NULL;
            while (stack_pop(context->context_task_graph_nodes,
                    (stack_item_t*) &graph_node))
                if (graph_node_has_children(graph_node) == false)
                    task_graph_add_edge(graph_node, context->context_end_node);
        }
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

    /* get enclosing context */
    region_context_t *context = NULL;
    stack_peek(thread_data->region_context_stack, (stack_item_t*) &context);

    /* make space for the newly-created task */
    task_data_t *task_data = new_task_data(get_unique_task_id(), flags, 0L);
    task_graph_node_type_t node_type = 
        flags & ompt_task_initial  ? node_task_initial  :
        flags & ompt_task_implicit ? node_task_implicit :
        flags & ompt_task_explicit ? node_task_explicit :
        flags & ompt_task_target   ? node_task_target   : node_type_unknown;

    /* create task graph node for this task */
    task_data->task_node_ref = task_graph_add_node(
        node_type, (task_graph_node_data_t) {.ptr = task_data});
    new_task->ptr = task_data;

    /* include task's node in the context's stack */
    pthread_mutex_lock(&context->lock);
    stack_push(context->context_task_graph_nodes,
        (stack_item_t) {.ptr = task_data->task_node_ref});
    pthread_mutex_unlock(&context->lock);

    /* 
        TODO:
        ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?
       give the task a reference to the context in which it was created so 
       child tasks (which may occur in other threads) can share the same context
            parent task is implicit -> get context from thread
            otherwise -> get context from parent task
        ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?
    */

    /* get the task data of the parent, if it exists */
    task_data_t *parent_task_data = NULL;

    if (encountering_task != NULL)
    {
        parent_task_data = (task_data_t*) encountering_task->ptr;

        if (parent_task_data->type == ompt_task_implicit)
        {
            /* Connect to enclosing context's begin node */
            task_graph_add_edge(context->context_begin_node,
                task_data->task_node_ref);
        } else {
            /* Connect to node of encountering task */
            task_graph_add_edge(parent_task_data->task_node_ref,
                task_data->task_node_ref);
        }
    }
    return;
}

static void
on_ompt_callback_task_schedule(
    ompt_data_t             *prior_task,
    ompt_task_status_t       prior_task_status,
    ompt_data_t             *next_task)
{
    LOG_DEBUG_PRIOR_TASK_STATUS(prior_task_status);

    task_data_t *prior_task_data = NULL, *next_task_data = NULL;

    #if DEBUG_LEVEL >= 3
    prior_task_data = (task_data_t*) prior_task->ptr;
    next_task_data = (task_data_t*) next_task->ptr;
    LOG_DEBUG("%lu, %lu", prior_task_data->id, next_task_data->id);
    #endif
    
    return;
}

/* 

   Events:
   initial-task-begin/initial-task-end
   implicit-task-begin/implicit-task-end

 */
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

    task_data_t *task_data = NULL;
    parallel_data_t *parallel_data = NULL;

    if (endpoint == ompt_scope_begin)
    {   
        /* Check whether task data is null */
        LOG_DEBUG("task pointer: %p->%p (flags=%d)", task, task->ptr, flags);

        /* Intel's runtime gives initial tasks task-create & implicit
         * -task-begin callbacks, but LLVM's only gives ITB callback
         * 
         * This means when using Intel runtime I allocate initial task
         * space in task-create, but with LLVM this happens below - need
         * to check for this to avoid double-counting an initial task
         */
        if (task->ptr != NULL)
        {
			LOG_WARN("task was previously allocated task data");
			task_data = (task_data_t*) task->ptr;
		} else {			
			/* make space for this initial or implicit task */
			task_data = new_task_data(get_unique_task_id(), flags, 0L);
            task->ptr = task_data;
		}

		LOG_DEBUG_IMPLICIT_TASK(flags, "begin", task_data->id);

        /* get the encompassing parallel region data (doesn't exist for initial
           tasks) and register this task in the task graph
         */
        if (task_data->type == ompt_task_initial)
        {
            /* an initial task has no enclosing context */
            // task_data->context = NULL;

            /* Add a node to the task graph for this initial task */
            task_data->task_node_ref = task_graph_add_node(
                node_task_initial, (task_graph_node_data_t) {.ptr = task_data}
            );
            thread_data->initial_task_graph_node_ref = task_data->task_node_ref;

        } else if (task_data->type == ompt_task_implicit) {

            /* get enclosing parallel context and push to the thread's context
               stack */
            parallel_data = (parallel_data_t*) parallel->ptr;
            stack_push(thread_data->region_context_stack,
                (stack_item_t) {.ptr = parallel_data->context});

            #if DEBUG_LEVEL >= 4
            stack_print(thread_data->region_context_stack);
            #endif

            /* the context of an implicit task is that of the enclosing parallel
               region (need this so that descendent tasks can also be aware of
               the enclosing context)
            */
            // task_data->context = parallel_data->context;
            
        } else {
            // Think this shouldn't happen
            fprintf(stderr, "UNEXPECTED IMPLICIT TASK CALLBACK");
            fprintf(stderr, "flags=%d actual_parallelism=%u index=%u",
                flags, actual_parallelism, index);
            abort();
        }

        /* register this task in the task graph */


    } else { /* ompt_scope_end */

        /* Pop the enclosing context from the thread's stack */
        stack_pop(thread_data->region_context_stack, NULL);

        #if DEBUG_LEVEL >= 4
        stack_print(thread_data->region_context_stack);
        #endif
    }

    return;
}

static void
on_ompt_callback_target(
    ompt_target_t            kind,
    ompt_scope_endpoint_t    endpoint,
    int                      device_num,
    ompt_data_t             *task,
    ompt_id_t                target_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_target_data_op(
    ompt_id_t                target_id,
    ompt_id_t                host_op_id,
    ompt_target_data_op_t    optype,
    void                    *src_addr,
    int                      src_device_num,
    void                    *dest_addr,
    int                      dest_device_num,
    size_t                   bytes,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_target_submit(
    ompt_id_t                target_id,
    ompt_id_t                host_op_id,
    unsigned int             requested_num_teams)
{
    return;
}

static void
on_ompt_callback_device_initialize(
    int                      device_num,
    const char              *type,
    ompt_device_t           *device,
    ompt_function_lookup_t   lookup,
    const char              *documentation)
{
    return;
}

static void
on_ompt_callback_device_finalize(
    int                      device_num)
{
    return;
}

static void
on_ompt_callback_device_load(
    int                      device_num,
    const char              *filename,
    int64_t                  offset_in_file,
    void                    *vma_in_file,
    size_t                   bytes,
    void                    *host_addr,
    void                    *device_addr,
    uint64_t                 module_id)
{
    return;
}

static void
on_ompt_callback_device_unload(
    int                      device_num,
    uint64_t                 module_id)
{
    return;
}

static void
on_ompt_callback_sync_region_wait(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_mutex_released(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_dependences(
    ompt_data_t             *task,
    const ompt_dependence_t *deps,
    int                      ndeps)
{
    return;
}

static void
on_ompt_callback_task_dependence(
    ompt_data_t             *src_task,
    ompt_data_t             *sink_task)
{
    return;
}

/* Used for callbacks that are dispatched when worksharing regions, loop-related
   regions, and taskloopregions begin and end.

   Events:
    section-begin/end (context=implicit task)
    single-begin/end
    workshare-begin/end (context=implicit task)
    ws-loop-begin/end (context=implicit task)
    distribute-begin/end (context=implicit task)
    taskloop-begin/end (context=encountering task)

   Inserting pseudo-tasks for workshare constructs
       - get encountering task data
       - create pseudo-task for the workshare region
       - use task_type enum from task_graph.h
       - attach ptask data inside encountering task data
       - register with task_graph as child of encountering task
       - when creating explicit task, first check whether encountering task
            has a ptask available - use as parent if available, otherwise don't
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
    task_data_t *workshare_task_data = NULL;
    
    LOG_DEBUG_WORK_TYPE(thread_data->id, wstype, count,
        endpoint==ompt_scope_begin?"begin":"end");

    region_context_t *context = NULL;
    if    ((wstype == ompt_work_single_executor)
        || (wstype == ompt_work_loop)
        || (wstype == ompt_work_taskloop))
    {
        if (endpoint == ompt_scope_begin)
        {
            context = malloc(sizeof(*context));
            context->type = 
                wstype == ompt_work_single_executor ? context_single :
                wstype == ompt_work_loop ? context_loop : 
                wstype == ompt_work_taskloop ? context_single : 0 ;
            context->context_data = NULL;
            stack_push(thread_data->region_context_stack,
                (stack_item_t) {.ptr = context});

        } else if (endpoint == ompt_scope_end) {
            stack_pop(thread_data->region_context_stack,
                (stack_item_t*) &context);
            free(context);
        }
        #if DEBUG_LEVEL >= 4
        stack_print(thread_data->region_context_stack);
        #endif
    }
    return;
}

static void
on_ompt_callback_master(
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_target_map(
    ompt_id_t                target_id,
    unsigned int             nitems,
    void *                  *host_addr,
    void *                  *device_addr,
    size_t                  *bytes,
    unsigned int            *mapping_flags,
    const void              *codeptr_ra)
{
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
    return;
}

static void
on_ompt_callback_lock_init(
    ompt_mutex_t             kind,
    unsigned int             hint,
    unsigned int             impl,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_lock_destroy(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_mutex_acquire(
    ompt_mutex_t             kind,
    unsigned int             hint,
    unsigned int             impl,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_mutex_acquired(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_nest_lock(
    ompt_scope_endpoint_t    endpoint,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_flush(
    ompt_data_t             *thread,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_cancel(
    ompt_data_t             *task,
    int                      flags,
    const void              *codeptr_ra)
{
    return;
}

static void
on_ompt_callback_reduction(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra)
{
    return;
}

static task_data_t *
new_task_data(
    unique_id_t      id,
    ompt_task_flag_t flags,
    unique_id_t      parallel)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id         = id,
        .type       = flags & OMPT_TASK_TYPE_BITS,
        .lock       = NULL,
        .enclosing_parallel_id = parallel,
        .workshare_child_task = NULL
        // .context = NULL
    };
    return new;
}

static unique_id_t
get_unique_id(
    unique_id_type_t         id_type)
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
