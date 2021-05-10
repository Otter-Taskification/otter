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

#include <otter-datatypes/queue.h>
#include <otter-datatypes/stack.h>

#define WORK_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_work_loop                                                     \
        ? attr_label_ref[attr_region_type_loop]             :                  \
    type == ompt_work_sections                                                 \
        ? attr_label_ref[attr_region_type_sections]         :                  \
    type == ompt_work_single_executor                                          \
        ? attr_label_ref[attr_region_type_single_executor]  :                  \
    type == ompt_work_single_other                                             \
        ? attr_label_ref[attr_region_type_single_other]     :                  \
    type == ompt_work_workshare                                                \
        ? attr_label_ref[attr_region_type_workshare]        :                  \
    type == ompt_work_distribute                                               \
        ? attr_label_ref[attr_region_type_distribute]       :                  \
    type == ompt_work_taskloop                                                 \
        ? attr_label_ref[attr_region_type_taskloop]         : 0)

#define WORK_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_work_loop             ? OTF2_REGION_ROLE_LOOP      :          \
    type == ompt_work_sections         ? OTF2_REGION_ROLE_SECTIONS  :          \
    type == ompt_work_single_executor  ? OTF2_REGION_ROLE_SINGLE    :          \
    type == ompt_work_single_other     ? OTF2_REGION_ROLE_SINGLE    :          \
    type == ompt_work_workshare        ? OTF2_REGION_ROLE_WORKSHARE :          \
    type == ompt_work_distribute       ? OTF2_REGION_ROLE_UNKNOWN   :          \
    type == ompt_work_taskloop         ? OTF2_REGION_ROLE_LOOP      :          \
        OTF2_REGION_ROLE_UNKNOWN)

/* Intel compiler defines additional types of sync regions compared to LLVM */
#if defined(__INTEL_COMPILER)
#define SYNC_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_sync_region_barrier                                           \
        ? attr_label_ref[attr_region_type_barrier]                  :          \
    type == ompt_sync_region_barrier_implicit                                  \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_explicit                                  \
        ? attr_label_ref[attr_region_type_barrier_explicit]         :          \
    type == ompt_sync_region_barrier_implementation                            \
        ? attr_label_ref[attr_region_type_barrier_implementation]   :          \
    type == ompt_sync_region_taskwait                                          \
        ? attr_label_ref[attr_region_type_taskwait]                 :          \
    type == ompt_sync_region_taskgroup                                         \
        ? attr_label_ref[attr_region_type_taskgroup]                :          \
    type == ompt_sync_region_barrier_implicit_workshare                        \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_implicit_parallel                         \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_teams                                     \
        ? attr_label_ref[attr_region_type_barrier_implicit]         : 0 )
#else
#define SYNC_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_sync_region_barrier                                           \
        ? attr_label_ref[attr_region_type_barrier]                  :          \
    type == ompt_sync_region_barrier_implicit                                  \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_explicit                                  \
        ? attr_label_ref[attr_region_type_barrier_explicit]         :          \
    type == ompt_sync_region_barrier_implementation                            \
        ? attr_label_ref[attr_region_type_barrier_implementation]   :          \
    type == ompt_sync_region_taskwait                                          \
        ? attr_label_ref[attr_region_type_taskwait]                 :          \
    type == ompt_sync_region_taskgroup                                         \
        ? attr_label_ref[attr_region_type_taskgroup] : 0)
#endif

#if defined(__INTEL_COMPILER)
#define SYNC_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_sync_region_barrier                                           \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implicit                                  \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_explicit                                  \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implementation                            \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_taskwait                                          \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_taskgroup                                         \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_barrier_implicit_workshare                        \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_implicit_parallel                         \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_teams                                     \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :  OTF2_REGION_ROLE_UNKNOWN)
#else
#define SYNC_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_sync_region_barrier                                           \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implicit                                  \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_explicit                                  \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implementation                            \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_taskwait                                          \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_taskgroup                                         \
        ? OTF2_REGION_ROLE_TASK_WAIT : OTF2_REGION_ROLE_UNKNOWN)
#endif

/* Different kinds of regions supported */
typedef enum {
    trace_region_parallel,
    trace_region_workshare,
    trace_region_synchronise
} trace_region_type_t;

/* Different kinds of unique IDs */
typedef enum trace_ref_type_t {
    trace_region,
    trace_string,
    trace_location,
    trace_other,
    NUM_REF_TYPES // <- MUST BE LAST ENUM ITEM
} trace_ref_type_t;

#define get_unique_rgn_ref() ((uint32_t) get_unique_ref(trace_region))
#define get_unique_str_ref() ((uint32_t) get_unique_ref(trace_string))
#define get_unique_loc_ref() (get_unique_ref(trace_location))
#define get_other_ref()      (get_unique_ref(trace_other))

/* Forward definitions */
typedef struct trace_parallel_region_attr_t trace_parallel_region_attr_t;
typedef struct trace_region_attr_empty_t    trace_region_attr_empty_t;
typedef struct trace_wshare_region_attr_t   trace_wshare_region_attr_t;
typedef struct trace_sync_region_attr_t     trace_sync_region_attr_t;

/* Attributes of a parallel region */
struct trace_parallel_region_attr_t {
    unique_id_t     id;
    unique_id_t     master_thread;
    bool            is_league;
    unsigned int    requested_parallelism;
    unsigned int    ref_count;
    pthread_mutex_t lock_rgn;
};

/* Attributes of a workshare region */
struct trace_wshare_region_attr_t {
    ompt_work_t     type;
    uint64_t        count;
};

/* Attributes of a sync region */
struct trace_sync_region_attr_t {
    ompt_sync_region_t  type;
    unique_id_t         encountering_task_id;
};

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
struct trace_region_def_t {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    union {
        trace_parallel_region_attr_t parallel;
        trace_wshare_region_attr_t   wshare;
        trace_sync_region_attr_t     sync;
    } attr;
};

/* Store values needed to register location definition (threads) with OTF2 */
struct trace_location_def_t {
    unique_id_t             id;
    ompt_thread_t           thread_type;
    uint64_t                events;
    stack_t                *rgn_stack;
    OTF2_EvtWriter         *evt_writer;
    OTF2_DefWriter         *def_writer;
    OTF2_LocationRef        ref;
    OTF2_LocationType       type;
    OTF2_LocationGroupRef   location_group;
    OTF2_AttributeList     *attributes;
};

static uint64_t get_unique_ref(trace_ref_type_t ref_type);

static uint64_t get_timestamp(void);

/* process a single location definition */
static void trace_write_location_definition(trace_location_def_t *loc);

/* Enum values are used as an index to lookup string refs for a given attribute
   name & label */
typedef enum {
#define INCLUDE_ATTRIBUTE(Type, Name, Desc) attr_##Name,
#include <otter-trace/trace-attribute-defs.h>
    n_attr_defined
} attr_name_enum_t;

typedef enum {
#define INCLUDE_LABEL(Name, Label) attr_##Name##_##Label,
#include <otter-trace/trace-attribute-defs.h>
    n_attr_label_defined
} attr_label_enum_t;

/* Global lookup tables mapping enum value to string ref */
static OTF2_StringRef attr_name_ref[n_attr_defined][2] = {0};
static OTF2_StringRef attr_label_ref[n_attr_label_defined] = {0};
/* References to global archive & def writer */
static OTF2_Archive *Archive = NULL;
static OTF2_GlobalDefWriter *Defs = NULL;

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

/* Initialise OTF2 archive
    - create archive & event files
    - set various options for OTF2
    - write some initial global definitions
    - populate attribute name & label string ref lookup tables
    - write attribute definitions which can then be referenced using attribute
      enum values
 */
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
trace_finalise_archive()
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

trace_location_def_t *
trace_new_location_definition(
    unique_id_t            id,
    ompt_thread_t          thread_type,
    OTF2_LocationType      loc_type,
    OTF2_LocationGroupRef  loc_grp)
{
    trace_location_def_t *new = malloc(sizeof(*new));

    *new = (trace_location_def_t) {
        .id             = id,
        .thread_type    = thread_type,
        .events         = 0,
        .ref            = get_unique_loc_ref(),
        .type           = loc_type,
        .location_group = loc_grp,
        .rgn_stack      = stack_create(),
        .attributes     = OTF2_AttributeList_New()
    };

    new->evt_writer = OTF2_Archive_GetEvtWriter(Archive, new->ref);
    new->def_writer = OTF2_Archive_GetDefWriter(Archive, new->ref);

    /* Thread location definition is written at thread-end (once all events
       counted) */

    return new;
}

trace_region_def_t *
trace_new_parallel_region(
    unique_id_t           id, 
    unique_id_t           master, 
    int                   flags,
    unsigned int          requested_parallelism)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = OTF2_REGION_ROLE_PARALLEL,
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_parallel,
        .attr.parallel = {
            .id            = id,
            .master_thread = master,
            .is_league     = flags & ompt_parallel_league ? true : false,
            .requested_parallelism = requested_parallelism,
            .ref_count     = 0,
            .lock_rgn      = PTHREAD_MUTEX_INITIALIZER
        }
    };

    /* Write a new region definition for each parallel region encountered */
    char region_name[DEFAULT_NAME_BUF_SZ+1] = {0};
    snprintf(region_name, DEFAULT_NAME_BUF_SZ, "Parallel Region %lu",
        new->attr.parallel.id);
    OTF2_StringRef region_name_ref = get_unique_str_ref();
    OTF2_GlobalDefWriter_WriteString(Defs,
        region_name_ref,
        region_name);
    OTF2_GlobalDefWriter_WriteRegion(Defs,
        new->ref,
        region_name_ref,
        0, 0,   /* canonical name, description */
        new->role,
        OTF2_PARADIGM_OPENMP,
        OTF2_REGION_FLAG_NONE,
        0, 0, 0); /* source file, begin line no., end line no. */
    return new;
}

void
trace_destroy_parallel_region(trace_region_def_t *rgn)
{
    LOG_DEBUG("destroying parallel region data (ref count is %u)",
        rgn->attr.parallel.ref_count);
    OTF2_AttributeList_Delete(rgn->attributes);
    free(rgn);
}

trace_region_def_t *
trace_new_workshare_region(
    ompt_work_t           wstype, 
    uint64_t              count)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = WORK_TYPE_TO_OTF2_REGION_ROLE(wstype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_workshare,
        .attr.wshare = {
            .type       = wstype,
            .count      = count
        }
    };

    /* Each workshare region encountered by a thread gets a unique definition */
    OTF2_GlobalDefWriter_WriteRegion(Defs,
        new->ref,
        WORK_TYPE_TO_STR_REF(new->attr.wshare.type),
        0, 0,
        new->role,
        OTF2_PARADIGM_OPENMP,
        OTF2_REGION_FLAG_NONE,
        0, 0, 0); /* source file, begin line no., end line no. */
    return new;
}

trace_region_def_t *
trace_new_sync_region(
    ompt_sync_region_t    stype, 
    unique_id_t           encountering_task_id)
{
    trace_region_def_t *new = malloc(sizeof(*new));
    *new = (trace_region_def_t) {
        .ref        = get_unique_rgn_ref(),
        .role       = SYNC_TYPE_TO_OTF2_REGION_ROLE(stype),
        .attributes = OTF2_AttributeList_New(),
        .type       = trace_region_synchronise,
        .attr.sync = {
            .type = stype,
            .encountering_task_id = encountering_task_id
        }
    };

    /* Each sync region encountered by a thread gets a unique definition */
    OTF2_GlobalDefWriter_WriteRegion(Defs,
        new->ref,
        SYNC_TYPE_TO_STR_REF(new->attr.sync.type),
        0, 0,
        new->role,
        OTF2_PARADIGM_OPENMP,
        OTF2_REGION_FLAG_NONE,
        0, 0, 0); /* source file, begin line no., end line no. */
    return new;
}

static void
trace_write_location_definition(trace_location_def_t *loc)
{
    if (loc == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }
    char location_name[DEFAULT_NAME_BUF_SZ + 1] = {0};
    OTF2_StringRef location_name_ref = get_unique_str_ref();
    snprintf(location_name, DEFAULT_NAME_BUF_SZ, "Thread %lu", loc->id);
    OTF2_GlobalDefWriter_WriteString(Defs,
        location_name_ref,
        location_name);
    OTF2_GlobalDefWriter_WriteLocation(Defs,
        loc->ref,
        location_name_ref,
        loc->type,
        loc->events,
        loc->location_group);
    free(loc);
    return;
}

void 
trace_event_thread(trace_location_def_t *self, ompt_scope_endpoint_t endpoint)
{
    /* Populate thread's attribute list for event */
    OTF2_AttributeList_AddStringRef(self->attributes, attr_thread_type,
        self->thread_type == ompt_thread_initial ? 
            attr_label_ref[attr_thread_type_initial] :
        self->thread_type == ompt_thread_worker ? 
            attr_label_ref[attr_thread_type_worker] : 0);
        
    if (endpoint == ompt_scope_begin)
        OTF2_EvtWriter_ThreadBegin(self->evt_writer,
            self->attributes, get_timestamp(), DEFAULT_COMM_REF, self->id);
    else
        OTF2_EvtWriter_ThreadEnd(self->evt_writer,
            self->attributes, get_timestamp(), DEFAULT_COMM_REF, self->id);

    self->events++;

    if (endpoint == ompt_scope_end) trace_write_location_definition(self);

    return;
}

void trace_event(
    trace_location_def_t  *self,
    trace_region_def_t    *region, 
    ompt_scope_endpoint_t  endpoint)
{
    LOG_ERROR_IF((endpoint == ompt_scope_begin && region == NULL),
        "Region must be given for region-begin event");

    #if DEBUG_LEVEL >= 4
    stack_print(self->rgn_stack);
    #endif

    /* For the region-end event, assume the region was previously pushed onto
       the location's region stack and is now at the top */
    if (endpoint == ompt_scope_end) 
        stack_pop(self->rgn_stack, (data_item_t*) &region);

    /* Add region's attributes to the enter/leave event */
    switch (region->type)
    {
        case trace_region_parallel:
    /* Parallel regions must be accessed atomically as they are shared between
       threads */
    LOG_DEBUG("acquiring mutex");
    pthread_mutex_lock(&region->attr.parallel.lock_rgn);
    LOG_DEBUG("acquired mutex");
    OTF2_AttributeList_AddUint64(region->attributes, attr_unique_id,
        region->attr.parallel.id);
    OTF2_AttributeList_AddUint32(region->attributes, attr_requested_parallelism,
        region->attr.parallel.requested_parallelism);
    OTF2_AttributeList_AddStringRef(region->attributes, attr_is_league,
        region->attr.parallel.is_league ? 
            attr_label_ref[attr_flag_true] : attr_label_ref[attr_flag_false]);
        break;

        case trace_region_workshare:
    OTF2_AttributeList_AddStringRef(region->attributes, attr_workshare_type,
        WORK_TYPE_TO_STR_REF(region->attr.wshare.type));
    OTF2_AttributeList_AddUint64(region->attributes, attr_workshare_count,
        region->attr.wshare.count);
        break;

        case trace_region_synchronise:
    OTF2_AttributeList_AddStringRef(region->attributes, attr_sync_type,
        SYNC_TYPE_TO_STR_REF(region->attr.sync.type));
    OTF2_AttributeList_AddUint64(region->attributes,
        attr_sync_encountering_task_id, region->attr.sync.encountering_task_id);
        break;

        default:
            LOG_ERROR("Unknown region type %d", region->type);
    }
    
    /* Record the event. For parallel regions, update the ref count to reflect
       the number of locations currently in the region */
    if (endpoint == ompt_scope_begin)
    {
        OTF2_EvtWriter_Enter(self->evt_writer, region->attributes, 
            get_timestamp(), region->ref);
        if (region->type == trace_region_parallel)
        {
            region->attr.parallel.ref_count++;
            LOG_DEBUG("ref count is %u - releasing mutex",
                region->attr.parallel.ref_count);
            pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
        }
        stack_push(self->rgn_stack, (data_item_t) {.ptr = region});
    } else {
        OTF2_EvtWriter_Leave(self->evt_writer, region->attributes,
            get_timestamp(), region->ref);
        if (region->type == trace_region_parallel)
        {
            region->attr.parallel.ref_count--;
            LOG_DEBUG("ref count is %u - releasing mutex",
                region->attr.parallel.ref_count);
            if (region->attr.parallel.ref_count == 0)
            {
                pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
                trace_destroy_parallel_region(region);
            } else {
                pthread_mutex_unlock(&region->attr.parallel.lock_rgn);                
            }
        } else {
            OTF2_AttributeList_Delete(region->attributes);
            free(region);
        }
    }
    self->events++;
    return;
}

static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}

static uint64_t
get_unique_ref(trace_ref_type_t ref_type)
{
    static uint64_t id[NUM_REF_TYPES] = {0};
    return __sync_fetch_and_add(&id[ref_type], 1L);
}