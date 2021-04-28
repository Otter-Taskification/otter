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
#include <otter-core/ompt-common.h>
#include <otter-trace/trace.h>

#include <otter-dtypes/queue.h>

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

static uint64_t get_unique_ref(trace_ref_type_t ref_type);

static uint64_t get_timestamp(void);

/* process a single location or region definition */
static void trace_write_location_definition(void *ptr);
static void trace_write_region_definition(void *ptr);

/* Enum values are used as an index to lookup string refs for a given attribute
   name & label
 */
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

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2
*/
typedef struct trace_region_def_t {

    /* Otter unique id */
    unique_id_t id;

    OTF2_RegionRef ref;
    OTF2_RegionRole role;
    OTF2_AttributeList *attributes;
} trace_region_def_t;

/* Store values needed to register location definition (threads) with OTF2
*/
typedef struct trace_location_def_t {

    /* Otter unique id */
    unique_id_t id;
    
    OTF2_EvtWriter *evt_writer;
    uint64_t events;
    OTF2_LocationRef ref;
    OTF2_LocationType type;
    OTF2_LocationGroupRef location_group;
} trace_location_def_t;

/* References to global archive & def writer */
static OTF2_Archive *Archive = NULL;
static OTF2_GlobalDefWriter *Defs = NULL;

/* Global data structures used to record definitions which are written out at
   program exit. Accessed w/ mtx
*/
static queue_t *trace_location_queue = NULL;
static pthread_mutex_t trace_location_queue_lock = PTHREAD_MUTEX_INITIALIZER;
static queue_t *trace_region_queue = NULL;
static pthread_mutex_t trace_region_queue_lock = PTHREAD_MUTEX_INITIALIZER;

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
    Archive = OTF2_Archive_Open("default-archive-path",
        "default-archive-name",
        OTF2_FILEMODE_WRITE,
        1024 * 1024,     /* event chunk size */
        4 * 1024 * 1024, /* def chunk size */
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
        UINT64_MAX); /* length */

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
       label
    */
    #define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_name_ref[attr_##Name][0], #Name);                       \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_name_ref[attr_##Name][1], #Desc);
    #define INCLUDE_LABEL(Name, Label)                                         \
        OTF2_GlobalDefWriter_WriteString(                                      \
            Defs, attr_label_ref[attr_##Name##_##Label], #Label);
    #include <otter-trace/trace-attribute-defs.h>

    /* define attributes which can be referred to later by the enum 
       attr_name_enum_t
    */
    #define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                \
        OTF2_GlobalDefWriter_WriteAttribute(Defs, attr_##Name,                 \
            attr_name_ref[attr_##Name][0],                                     \
            attr_name_ref[attr_##Name][1],                                     \
            Type);
    #include <otter-trace/trace-attribute-defs.h>

    /* Initialise location & definition queues with destructors for each
       definition in the queue
     */
    trace_location_queue = queue_create(&trace_write_location_definition);
    trace_region_queue = queue_create(&trace_write_region_definition);

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
    for (int loc = 0; loc < nloc; loc++)
    {
        OTF2_DefWriter* dw = OTF2_Archive_GetDefWriter(Archive, loc);
        OTF2_Archive_CloseDefWriter(Archive, dw);
    }

    /* close local definition files */
    OTF2_Archive_CloseDefFiles(Archive);

    /* * * * WRITE   LOCATION   DEFINITIONS * * * */
    // trace_location_def_t *loc = NULL;
    // queue_item_t *item = NULL;
    // int location_count = 0;
    // int event_count = 0;
    // char location_name[32] = {0};

    // while (queue_pop(trace_location_queue, &item))
    // {
    //     loc = (trace_location_def_t*) item->ptr;
    //     OTF2_StringRef location_name_ref = get_unique_str_ref();
    //     snprintf(location_name, 32, "Thread %lu", loc->ref);
    //     OTF2_GlobalDefWriter_WriteString(Defs,
    //         location_name_ref,
    //         location_name);
    //     OTF2_GlobalDefWriter_WriteLocation(Defs,
    //         loc->ref,
    //         location_name_ref,
    //         loc->type,
    //         loc->events,
    //         loc->location_group);
    //     event_count += loc->events;
    //     free(loc);
    //     location_count++;
    // }

    /* * * * WRITE   REGION   DEFINITIONS * * * */
    // trace_region_def_t *rgn = NULL;
    // int region_count = 0;
    // char region_name[32] = {0};

    // while ((rgn = (trace_region_def_t*) queue_pop(&g_region_queue)) != NULL)
    // while (queue_pop(trace_region_queue, &item))
    // {
    //     rgn = (trace_region_def_t*) item->ptr;
    //     OTF2_StringRef region_name_ref = get_unique_str_ref();
    //     snprintf(region_name, 32, "Region %u", rgn->ref);
    //     OTF2_GlobalDefWriter_WriteString(Defs,
    //         region_name_ref,
    //         region_name);
    //     OTF2_GlobalDefWriter_WriteRegion(Defs,
    //         rgn->ref,
    //         region_name_ref,
    //         0, 0,   /* canonical name, description */
    //         rgn->role,
    //         OTF2_PARADIGM_OPENMP,
    //         OTF2_REGION_FLAG_NONE,
    //         0, 0, 0); /* source file, begin line no., end line no. */
    //     free(rgn);
    //     region_count++;
    // }

    // fprintf(stderr, "\nDEFINITIONS RECORDED:\n"
    //                 "%12s: %6d\n"
    //                 "%12s: %6d\n"
    //                 "%12s: %6d\n"
    //                 "%12s: %6d\n"
    //                 "%12s: %6d\n"
    //                 "%12s: %6u\n",
    //                 "events",     event_count,
    //                 "locations",  location_count,
    //                 "regions",    region_count,
    //                 "strings",    get_unique_str_ref(),
    //                 "attributes", n_attr_defined,
    //                 "labels",     n_attr_label_defined);

    /* Destroy location and region queues - destructor takes care of
       writing to archive
     */
    queue_destroy(trace_location_queue, true);
    queue_destroy(trace_region_queue, true);

    /* close OTF2 archive */
    OTF2_Archive_Close(Archive);

    return true;
}

static void
trace_write_location_definition(void *ptr)
{
    if (ptr == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }
    trace_location_def_t *loc = (trace_location_def_t*) ptr;
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

static void
trace_write_region_definition(void *ptr)
{
    if (ptr == NULL)
    {
        LOG_ERROR("null pointer");
        return;
    }
    trace_region_def_t *rgn = (trace_region_def_t*) ptr;
    char region_name[DEFAULT_NAME_BUF_SZ] = {0};
    OTF2_StringRef region_name_ref = get_unique_str_ref();
    snprintf(region_name, DEFAULT_NAME_BUF_SZ, "Region %lu", rgn->id);
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
    free(rgn);
    return;
}

trace_location_def_t *
trace_new_location_definition(
    uint64_t               id,
    OTF2_LocationType      loc_type,
    OTF2_LocationGroupRef  loc_grp)
{
    trace_location_def_t *new = malloc(sizeof(*new));
    new->id = id;
    new->events = 0;
    new->ref = get_unique_loc_ref();
    new->type = loc_type;
    new->location_group = loc_grp;
    new->evt_writer = OTF2_Archive_GetEvtWriter(Archive, new->ref);

    /* add location definition to global queue */
    pthread_mutex_lock(&trace_location_queue_lock);
    queue_push(trace_location_queue, (queue_item_t) {.ptr = new});
    pthread_mutex_unlock(&trace_location_queue_lock);

    return new;
}

trace_region_def_t *
trace_new_region_definition(
    uint64_t           id,
    OTF2_RegionRole    rgn_role)
{
    trace_region_def_t *new = malloc(sizeof(*new));

    new->id = id;
    new->ref = get_unique_rgn_ref();
    new->role = rgn_role;
    new->attributes = NULL;

    /* add region definition data to global queue */
    pthread_mutex_lock(&trace_region_queue_lock);
    queue_push(trace_region_queue, (queue_item_t) {.ptr = new});
    pthread_mutex_unlock(&trace_region_queue_lock);

    return new;
}

static uint64_t
get_unique_ref(trace_ref_type_t ref_type)
{
    static uint64_t id[NUM_REF_TYPES] = {0};
    return __sync_fetch_and_add(&id[ref_type], 1L);
}

void
trace_event_thread_begin(trace_location_def_t *self)
{
    OTF2_EvtWriter_ThreadBegin(self->evt_writer,
        NULL, get_timestamp(), DEFAULT_COMM_REF, self->id);
    self->events++;
    return;
}

void
trace_event_thread_end(trace_location_def_t *self)
{
    OTF2_EvtWriter_ThreadEnd(self->evt_writer,
        NULL, get_timestamp(), DEFAULT_COMM_REF, OTF2_UNDEFINED_UINT64);
    self->events++;
    return;
}

void 
trace_event_enter(
    trace_location_def_t  *self,
    trace_region_def_t    *region)
{
    OTF2_EvtWriter_Enter(self->evt_writer,
        NULL, get_timestamp(), region->ref);
    self->events++;
    return;
}

void 
trace_event_leave(
    trace_location_def_t  *self,
    trace_region_def_t    *region)
{
    OTF2_EvtWriter_Leave(self->evt_writer,
        NULL, get_timestamp(), region->ref);
    return;
}


static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}
