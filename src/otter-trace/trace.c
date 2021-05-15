#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <otf2/otf2.h>
#include <otf2/OTF2_Pthread_Locks.h>

#include <macros/debug.h>
#include <otter-common.h>
#include <otter-trace/trace.h>
#include <otter-trace/trace-structs.h>
#include <otter-trace/trace-attributes.h>
#include <otter-trace/trace-lookup-macros.h>

#include <otter-datatypes/queue.h>
#include <otter-datatypes/stack.h>

static uint64_t get_timestamp(void);

/* apply a region's attributes to an event */
static void trace_add_thread_attributes(trace_location_def_t *self);
static void trace_add_parallel_attributes(trace_region_def_t *rgn);
static void trace_add_workshare_attributes(trace_region_def_t *rgn);
static void trace_add_sync_attributes(trace_region_def_t *rgn);
static void trace_add_task_attributes(trace_region_def_t *rgn);

/* Lookup tables mapping enum value to string ref */
static OTF2_StringRef attr_name_ref[n_attr_defined][2] = {0};
static OTF2_StringRef attr_label_ref[n_attr_label_defined] = {0};

/* References to global archive & def writer */
OTF2_Archive *Archive = NULL;
OTF2_GlobalDefWriter *Defs = NULL;

/* Mutexes for thread-safe access to Archive and Defs */
pthread_mutex_t lock_global_def_writer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_global_archive    = PTHREAD_MUTEX_INITIALIZER;

/* Pre- and post-flush callbacks required by OTF2 */
static OTF2_FlushType
pre_flush(
    void               *userData,
    OTF2_FileType       fileType,
    OTF2_LocationRef    location,
    void               *callerData,
    bool                final)
{
    return OTF2_FLUSH;
}

static OTF2_TimeStamp
post_flush(
    void               *userData,
    OTF2_FileType       fileType,
    OTF2_LocationRef    location)
{
    return get_timestamp();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   INITIALISE/FINALISE TRACING                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
trace_initialise_archive(otter_opt_t *opt)
{
    /* open OTF2 archive */
    Archive = OTF2_Archive_Open(
        "default-archive-path",
        "default-archive-name",
        OTF2_FILEMODE_WRITE,
        1024 * 1024,                     /* event chunk size */
        4 * 1024 * 1024,                 /* def chunk size */
        OTF2_SUBSTRATE_POSIX,
        OTF2_COMPRESSION_NONE);

    /* set flush callbacks */
    static OTF2_FlushCallbacks on_flush = {
        .otf2_pre_flush = pre_flush,
        .otf2_post_flush = post_flush
    };
    OTF2_Archive_SetFlushCallbacks(Archive, &on_flush, NULL);

    /* set serial (not MPI) collective callbacks */
    OTF2_Archive_SetSerialCollectiveCallbacks(Archive);

    /* set pthread archive locking callbacks */
    OTF2_Pthread_Archive_SetLockingCallbacks(Archive, NULL);

    /* open archive event files */
    OTF2_Archive_OpenEvtFiles(Archive);

    /* open (thread-) local definition files */
    OTF2_Archive_OpenDefFiles(Archive);

    /* get global definitions writer */
    Defs = OTF2_Archive_GetGlobalDefWriter(Archive);

    /* get clock resolution & current time for CLOCK_MONOTONIC */
    struct timespec res, tp;
    if (clock_getres(CLOCK_MONOTONIC, &res) != 0)
    {
        LOG_ERROR("%s", strerror(errno));
        errno = 0;
    } else {
        LOG_DEBUG("Clock resolution: %lu s", res.tv_sec);
        LOG_DEBUG("Clock resolution: %lu ns", res.tv_nsec);
        LOG_DEBUG("Clock ticks per second: %lu", 1000000000 / res.tv_nsec);
    }

    clock_gettime(CLOCK_MONOTONIC, &tp);
    uint64_t epoch = tp.tv_sec * (uint64_t)1000000000 + tp.tv_nsec;
    LOG_DEBUG("Epoch: %lu %lu %lu", tp.tv_sec, tp.tv_nsec, epoch);

    /* write global clock properties */
    OTF2_GlobalDefWriter_WriteClockProperties(Defs,
        1000000000 / res.tv_nsec,  /* ticks per second */
        epoch,
        UINT64_MAX                 /* length */
    );

    /* write an empty string as the first entry so that string ref 0 is "" */
    OTF2_GlobalDefWriter_WriteString(Defs, get_unique_str_ref(), "");

    /* write global system tree */
    OTF2_SystemTreeNodeRef g_sys_tree_id = DEFAULT_SYSTEM_TREE;
    OTF2_StringRef g_sys_tree_name = get_unique_str_ref();
    OTF2_StringRef g_sys_tree_class = get_unique_str_ref();
    OTF2_GlobalDefWriter_WriteString(Defs, g_sys_tree_name, "Sytem Tree");
    OTF2_GlobalDefWriter_WriteString(Defs, g_sys_tree_class, "node");
    OTF2_GlobalDefWriter_WriteSystemTreeNode(Defs,
        g_sys_tree_id,
        g_sys_tree_name,
        g_sys_tree_class,
        OTF2_UNDEFINED_SYSTEM_TREE_NODE);

    /* write global location group */
    OTF2_StringRef g_loc_grp_name = get_unique_str_ref();
    OTF2_LocationGroupRef g_loc_grp_id = DEFAULT_LOCATION_GRP;
    OTF2_GlobalDefWriter_WriteString(Defs, g_loc_grp_name, "OMP Process");
    OTF2_GlobalDefWriter_WriteLocationGroup(Defs, g_loc_grp_id, g_loc_grp_name,
        OTF2_LOCATION_GROUP_TYPE_PROCESS, g_sys_tree_id);

    /* define any necessary attributes (their names, descriptions & labels)
       these are defined in trace-attribute-defs.h and included via macros to
       reduce code repetition. */

    /* Populate lookup tables with unique string refs */
    int k=0;
    for (k=0; k<n_attr_defined; k++)
    {
        attr_name_ref[k][0] = get_unique_str_ref();
        attr_name_ref[k][1] = get_unique_str_ref();
    }

    for (k=0; k<n_attr_label_defined; k++)
        attr_label_ref[k] = get_unique_str_ref();

    /* read attributes from header and write name, description & label strings. 
       lookup the string refs using the enum value for a particular attribute &
       label */
    #define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_name_ref[attr_##Name][0], #Name);                       \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_name_ref[attr_##Name][1],  Desc);
    #define INCLUDE_LABEL(Name, Label)                                         \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_label_ref[attr_##Name##_##Label], #Label);
    #include <otter-trace/trace-attribute-defs.h>

    /* define attributes which can be referred to later by the enum 
       attr_name_enum_t */
    #define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                \
        OTF2_GlobalDefWriter_WriteAttribute(Defs, attr_##Name,                 \
            attr_name_ref[attr_##Name][0],                                     \
            attr_name_ref[attr_##Name][1],                                     \
            Type);
    #include <otter-trace/trace-attribute-defs.h>

    return true;
}

bool
trace_finalise_archive(void)
{
    /* close event files */
    OTF2_Archive_CloseEvtFiles(Archive);

    /* create 1 definition writer per location & immediately close it - not 
       currently used
     */
    uint64_t nloc = get_unique_loc_ref();
    int loc = 0;
    for (loc = 0; loc < nloc; loc++)
    {
        OTF2_DefWriter* dw = OTF2_Archive_GetDefWriter(Archive, loc);
        OTF2_Archive_CloseDefWriter(Archive, dw);
    }

    /* close local definition files */
    OTF2_Archive_CloseDefFiles(Archive);

    /* close OTF2 archive */
    OTF2_Archive_Close(Archive);

    return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE DEFINITIONS                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
trace_write_location_definition(trace_location_def_t *loc)
{
    if (loc == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }
    LOG_DEBUG("writing definition: location %lu (Otter id %lu)",
        loc->ref, loc->id);
    char location_name[DEFAULT_NAME_BUF_SZ + 1] = {0};
    OTF2_StringRef location_name_ref = get_unique_str_ref();
    snprintf(location_name, DEFAULT_NAME_BUF_SZ, "Thread %lu", loc->id);
    LOG_DEBUG("thread %lu acquiring lock_global_def_writer", loc->id);
    pthread_mutex_lock(&lock_global_def_writer);
    OTF2_GlobalDefWriter_WriteString(Defs,
        location_name_ref,
        location_name);
    OTF2_GlobalDefWriter_WriteLocation(Defs,
        loc->ref,
        location_name_ref,
        loc->type,
        loc->events,
        loc->location_group);
    pthread_mutex_unlock(&lock_global_def_writer);
    LOG_DEBUG("thread %lu releasing lock_global_def_writer", loc->id);
    return;
}

void
trace_write_region_definition(trace_region_def_t *rgn)
{
    if (rgn == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }
    LOG_DEBUG("writing definition for region %u (type=%d, role=%u) %p",
        rgn->ref, rgn->type, rgn->role, rgn);
    switch (rgn->type)
    {
        case trace_region_parallel:
        {
            char region_name[DEFAULT_NAME_BUF_SZ+1] = {0};
            snprintf(region_name, DEFAULT_NAME_BUF_SZ, "Parallel Region %lu",
                rgn->attr.parallel.id);
            OTF2_StringRef region_name_ref = get_unique_str_ref();
            OTF2_GlobalDefWriter_WriteString(Defs,
                region_name_ref,
                region_name);
            OTF2_GlobalDefWriter_WriteRegion(Defs,
                rgn->ref,
                region_name_ref,
                0, 0,   /* canonical name, description */
                rgn->role,
                OTF2_PARADIGM_OPENMP,
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_workshare:
        {
            OTF2_GlobalDefWriter_WriteRegion(Defs,
                rgn->ref,
                WORK_TYPE_TO_STR_REF(rgn->attr.wshare.type),
                0, 0,
                rgn->role,
                OTF2_PARADIGM_OPENMP,
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_synchronise:
        {
            OTF2_GlobalDefWriter_WriteRegion(Defs,
                rgn->ref,
                SYNC_TYPE_TO_STR_REF(rgn->attr.sync.type),
                0, 0,
                rgn->role,
                OTF2_PARADIGM_OPENMP,
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        case trace_region_task:
        {
            char task_name[DEFAULT_NAME_BUF_SZ+1] = {0};
            snprintf(task_name, DEFAULT_NAME_BUF_SZ, "%s task %lu",
                rgn->attr.task.type == ompt_task_initial ? "initial" :
                    rgn->attr.task.type == ompt_task_implicit ? "implicit" :
                    rgn->attr.task.type == ompt_task_explicit ? "explicit" :
                    rgn->attr.task.type == ompt_task_target   ? "target" : "??",
                rgn->attr.task.id);
            OTF2_StringRef task_name_ref = get_unique_str_ref();
            OTF2_GlobalDefWriter_WriteString(Defs, task_name_ref, task_name);
            OTF2_GlobalDefWriter_WriteRegion(Defs,
                rgn->ref,
                task_name_ref,
                0, 0,   /* canonical name, description */
                rgn->role,
                OTF2_PARADIGM_OPENMP,
                OTF2_REGION_FLAG_NONE,
                0, 0, 0); /* source file, begin line no., end line no. */
            break;
        }
        default:
        {
            LOG_ERROR("unexpected region type %d", rgn->type);
        }
    }
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   ADD LOCATION/REGION ATTRIBUTES BEFORE RECORDING EVENTS                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
trace_add_thread_attributes(trace_location_def_t *self)
{
    OTF2_AttributeList_AddStringRef(self->attributes, attr_thread_type,
        self->thread_type == ompt_thread_initial ? 
            attr_label_ref[attr_thread_type_initial] :
        self->thread_type == ompt_thread_worker ? 
            attr_label_ref[attr_thread_type_worker] : 0);
    return;
}

static void
trace_add_parallel_attributes(trace_region_def_t *rgn)
{
    OTF2_AttributeList_AddUint64(rgn->attributes, attr_unique_id,
        rgn->attr.parallel.id);
    OTF2_AttributeList_AddUint32(rgn->attributes, attr_requested_parallelism,
        rgn->attr.parallel.requested_parallelism);
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_is_league,
        rgn->attr.parallel.is_league ? 
            attr_label_ref[attr_flag_true] : attr_label_ref[attr_flag_false]);
    return;
}

static void
trace_add_workshare_attributes(trace_region_def_t *rgn)
{
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_workshare_type,
        WORK_TYPE_TO_STR_REF(rgn->attr.wshare.type));
    OTF2_AttributeList_AddUint64(rgn->attributes, attr_workshare_count,
        rgn->attr.wshare.count);
    return;
}

static void
trace_add_sync_attributes(trace_region_def_t *rgn)
{
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_sync_type,
        SYNC_TYPE_TO_STR_REF(rgn->attr.sync.type));
    OTF2_AttributeList_AddUint64(rgn->attributes, attr_encountering_task_id,
        rgn->attr.sync.encountering_task_id);
    return;
}

static void
trace_add_task_attributes(trace_region_def_t *rgn)
{
    OTF2_AttributeList_AddUint64(rgn->attributes, attr_unique_id,
        rgn->attr.task.id);
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_task_type,
        TASK_TYPE_TO_STR_REF(rgn->attr.task.type));
    OTF2_AttributeList_AddUint32(rgn->attributes, attr_task_flags,
        rgn->attr.task.flags);
    OTF2_AttributeList_AddUint64(rgn->attributes, attr_parent_task_id,
        rgn->attr.task.parent_id);
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_parent_task_type,
        TASK_TYPE_TO_STR_REF(rgn->attr.task.parent_type));
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_has_dependences,
        rgn->attr.task.has_dependences);
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_undeferred,
        rgn->attr.task.flags & ompt_task_undeferred);
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_untied,
        rgn->attr.task.flags & ompt_task_untied);
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_final,
        rgn->attr.task.flags & ompt_task_final);
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_mergeable,
        rgn->attr.task.flags & ompt_task_mergeable);
    OTF2_AttributeList_AddUint8(rgn->attributes, attr_task_is_merged,
        rgn->attr.task.flags & ompt_task_merged);
    OTF2_AttributeList_AddStringRef(rgn->attributes, attr_prior_task_status,
        TASK_STATUS_TO_STR_REF(rgn->attr.task.task_status));
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   WRITE EVENTS                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
trace_event_thread_begin(trace_location_def_t *self)
{
    trace_add_thread_attributes(self);
    OTF2_EvtWriter_ThreadBegin(
        self->evt_writer,
        self->attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        self->id
    );
    self->events++;
    return;
}

void
trace_event_thread_end(trace_location_def_t *self)
{
    trace_add_thread_attributes(self);
    OTF2_EvtWriter_ThreadEnd(
        self->evt_writer,    
        self->attributes, 
        get_timestamp(), 
        OTF2_UNDEFINED_COMM, 
        self->id
    );
    self->events++;
    return;
}

void
trace_event_enter(
    trace_location_def_t *self,
    trace_region_def_t *region)
{
    LOG_ERROR_IF((region == NULL), "null region pointer");

    #if DEBUG_LEVEL >= 4
    stack_print(self->rgn_stack);
    #endif

    if (region->type == trace_region_parallel)
    {
        /* Parallel regions must be accessed atomically as they are shared 
           between threads */
        LOG_DEBUG("(%3d) thread %lu acquiring mutex", __LINE__, self->id);
        pthread_mutex_lock(&region->attr.parallel.lock_rgn);
        LOG_DEBUG("(%3d) thread %lu acquired mutex", __LINE__, self->id);
    }

    /* Add the event type attribute */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_event_type,
        region->type == trace_region_parallel ?
            attr_label_ref[attr_event_type_parallel_begin] :
        region->type == trace_region_workshare ?
            attr_label_ref[attr_event_type_workshare_begin] :
        region->type == trace_region_synchronise ?
            attr_label_ref[attr_event_type_sync_begin] :
        attr_label_ref[attr_event_type_task_enter]
    );

    /* Add region's attributes to the event */
    switch (region->type)
    {
    case trace_region_parallel:
        trace_add_parallel_attributes(region);
    break;
    case trace_region_workshare:
        trace_add_workshare_attributes(region);
    break;
    case trace_region_synchronise:
        trace_add_sync_attributes(region);
    break;
    case trace_region_task:
        trace_add_task_attributes(region);
    break;
    default:
        LOG_ERROR("unhandled region type %d", region->type);
        abort();
    }
    
    /* Record the event */
    OTF2_EvtWriter_Enter(self->evt_writer, 
        region->attributes, get_timestamp(), region->ref);

    /* Push region onto location's region stack */
    stack_push(self->rgn_stack, (data_item_t) {.ptr = region});

    if (region->type == trace_region_parallel)
    {
        region->attr.parallel.ref_count++;
        LOG_DEBUG("(%3d) ref count of parallel region %u is %u - releasing "
            "mutex",  __LINE__, region->ref, region->attr.parallel.ref_count);
        pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
    }

    self->events++;
    return;
}

void
trace_event_leave(trace_location_def_t *self)
{
    #if DEBUG_LEVEL >= 4
    stack_print(self->rgn_stack);
    #endif

    /* For the region-end event, the region was previously pushed onto the 
       location's region stack so should now be at the top (as long as regions
       are correctly nested) */
    trace_region_def_t *region = NULL;
    stack_pop(self->rgn_stack, (data_item_t*) &region);

    if (region->type == trace_region_parallel)
    {
        /* Parallel regions must be accessed atomically as they are shared 
           between threads */
        LOG_DEBUG("(%3d) thread %lu acquiring mutex", __LINE__, self->id);
        pthread_mutex_lock(&region->attr.parallel.lock_rgn);
        LOG_DEBUG("(%3d) thread %lu acquired mutex", __LINE__, self->id);
    }

    /* Add the event type attribute */
    OTF2_AttributeList_AddStringRef(region->attributes, attr_event_type,
        region->type == trace_region_parallel ?
            attr_label_ref[attr_event_type_parallel_end] :
        region->type == trace_region_workshare ?
            attr_label_ref[attr_event_type_workshare_end] :
        region->type == trace_region_synchronise ?
            attr_label_ref[attr_event_type_sync_end] :
        attr_label_ref[attr_event_type_task_leave]
    );

    /* Add region's attributes to the event */
    switch (region->type)
    {
    case trace_region_parallel:
        trace_add_parallel_attributes(region);
    break;
    case trace_region_workshare:
        trace_add_workshare_attributes(region);
    break;
    case trace_region_synchronise:
        trace_add_sync_attributes(region);
    break;
    case trace_region_task:
        trace_add_task_attributes(region);
    break;
    default:
        LOG_ERROR("unhandled region type %d", region->type);
        abort();
    }

    /* Record the event */
    OTF2_EvtWriter_Leave(self->evt_writer, region->attributes, get_timestamp(),
        region->ref);
    
    /* Parallel regions must be cleaned up by the last thread to leave */
    if (region->type == trace_region_parallel)
    {
        /* Give the location's region definitions to the parallel region */
        LOG_DEBUG("(%3d) thread %lu appending queue of %lu region "
            "definitions to parallel region %lu", 
            __LINE__, self->id, queue_length(self->rgn_defs),
            region->attr.parallel.id);
        if (!queue_append(region->attr.parallel.rgn_defs, self->rgn_defs))
            LOG_ERROR("error appending items to queue");
        region->attr.parallel.ref_count--;
        LOG_DEBUG("(%3d) ref count of parallel region %u is %u",
            __LINE__, region->ref, region->attr.parallel.ref_count);

        /* Check the ref count atomically __before__ unlocking */
        if (region->attr.parallel.ref_count == 0)
        {
            trace_destroy_parallel_region(region);
        } else {
            LOG_DEBUG("(%3d) thread %lu releasing mutex",
                __LINE__, self->id);
            pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
        }
    }
    
    self->events++;
    return;
}

void
trace_event_task_create(
    trace_location_def_t *self, 
    trace_region_def_t   *created_task)
{
    OTF2_AttributeList_AddStringRef(created_task->attributes, attr_event_type,
        attr_label_ref[attr_event_type_task_create]);
    trace_add_task_attributes(created_task);
    OTF2_EvtWriter_ThreadTaskCreate(
        self->evt_writer,
        created_task->attributes,
        get_timestamp(),
        OTF2_UNDEFINED_COMM,
        OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
    self->events++;
    return;
}

void 
trace_event_task_schedule(
    trace_location_def_t *self,
    trace_region_def_t *prior_task,
    ompt_task_status_t prior_status)
{
    /* Update prior task's status before recording task enter/leave events */
    LOG_ERROR_IF((prior_task->type != trace_region_task),
        "invalid region type %d", prior_task->type);
    prior_task->attr.task.task_status = prior_status;
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  TIMESTAMP & UNIQUE REFERENCES                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}

uint64_t
get_unique_uint64_ref(trace_ref_type_t ref_type)
{
    static uint64_t id[NUM_REF_TYPES] = {0};
    return __sync_fetch_and_add(&id[ref_type], 1L);
}

uint32_t
get_unique_uint32_ref(trace_ref_type_t ref_type)
{
    static uint32_t id[NUM_REF_TYPES] = {0};
    return __sync_fetch_and_add(&id[ref_type], 1);
}
