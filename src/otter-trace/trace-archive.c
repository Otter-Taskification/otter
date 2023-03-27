/**
 * @file trace-archive.c
 * @author Adam Tuft
 * @brief Responsible for initialising and finalising single instances of the 
 * trace archive and its global definitions writer. Returns pointers to these 
 * resources as well as mutexes protecting access to them both.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <otf2/otf2.h>
#include <otf2/OTF2_Pthread_Locks.h>

#include "public/otter-version.h"
#include "public/debug.h"
#include "public/otter-trace/trace.h"
#include "src/otter-trace/trace-archive.h"
#include "private/otter-trace/trace-string-registry.h"
#include "src/otter-trace/trace-attributes.h"
#include "private/otter-trace/trace-unique-refs.h"
#include "private/otter-trace/trace-check-error-code.h"

#define DEFAULT_LOCATION_GRP 0 // OTF2_UNDEFINED_LOCATION_GROUP
#define DEFAULT_SYSTEM_TREE  0
#define DEFAULT_NAME_BUF_SZ  256

static uint64_t get_timestamp(void);

/* Lookup tables mapping enum value to string ref */
OTF2_StringRef attr_name_ref[n_attr_defined][2] = {0};
OTF2_StringRef attr_label_ref[n_attr_label_defined] = {0};

/* References to global archive & def writer */
static OTF2_Archive *Archive = NULL;
static OTF2_GlobalDefWriter *Defs = NULL;

/* Mutexes for thread-safe access to Archive and Defs */
static pthread_mutex_t lock_global_def_writer = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock_global_archive    = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t *
global_def_writer_lock(void)
{
    return &lock_global_def_writer;
}

pthread_mutex_t *
global_archive_lock(void)
{
    return &lock_global_archive;
}

OTF2_GlobalDefWriter *get_global_def_writer(void)
{
    return Defs;
}

OTF2_Archive *get_global_archive(void)
{
    return Archive;
}

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

// Callback to pass to Registry which will be used to write source location
// string refs to the DefWriter when Registry is deleted.
static void trace_write_string_ref(const char *s, OTF2_StringRef ref)
{
    LOG_DEBUG("[%s] writing ref %u for string \"%s\"", __func__, ref, s);
    OTF2_ErrorCode r = OTF2_SUCCESS;
    r = OTF2_GlobalDefWriter_WriteString(Defs, ref, s);
    CHECK_OTF2_ERROR_CODE(r);
}

bool
trace_initialise_archive(otter_opt_t *opt)
{
    OTF2_ErrorCode ret = OTF2_SUCCESS;

    /* Determine filename & path from options */
    char archive_path[DEFAULT_NAME_BUF_SZ+1] = {0};
    static char archive_name[DEFAULT_NAME_BUF_SZ+1] = {0};
    char *p = &archive_name[0];

    /* Copy filename */
    strncpy(p, opt->tracename, DEFAULT_NAME_BUF_SZ - strlen(archive_name));
    p = &archive_name[0] + strlen(archive_name);

    /* Copy hostname */
    if (opt->append_hostname)
    {
        strncpy(p, ".", DEFAULT_NAME_BUF_SZ - strlen(archive_name));
        p = &archive_name[0] + strlen(archive_name);
        strncpy(p, opt->hostname, DEFAULT_NAME_BUF_SZ - strlen(archive_name));
        p = &archive_name[0] + strlen(archive_name);
    }

    /* Copy PID */
    strncpy(p, ".", DEFAULT_NAME_BUF_SZ - strlen(archive_name));
    p = &archive_name[0] + strlen(archive_name);
    snprintf(p, DEFAULT_NAME_BUF_SZ - strlen(archive_name), "%u", getpid());
    p = &archive_name[0] + strlen(archive_name);

    /* Copy path + filename */
    snprintf(archive_path, DEFAULT_NAME_BUF_SZ, "%s/%s",
        opt->tracepath, archive_name);

    fprintf(stderr, "%-30s %s/%s\n",
        "Trace output path:", opt->tracepath, archive_name);

    /* Store archive name in options struct */
    opt->archive_name = &archive_name[0];

    /* open OTF2 archive */
    Archive = OTF2_Archive_Open(
        archive_path,               /* archive path */
        archive_name,               /* archive name */
        OTF2_FILEMODE_WRITE,
        OTF2_CHUNK_SIZE_EVENTS_DEFAULT,     
        OTF2_CHUNK_SIZE_DEFINITIONS_DEFAULT,
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

#if defined(OTTER_EVENT_MODEL_OMP) || defined(OTTER_EVENT_MODEL_SERIAL)
    // otter-serial uses the OMP event model
    const char* event_model = "OMP";
#elif defined(OTTER_EVENT_MODEL_TASKGRAPH)
    // otter-task-graph uses its own event model
    const char* event_model = "TASKGRAPH";
#else
    const char* event_model = "UNKNOWN";
#endif

    ret = OTF2_Archive_SetProperty(Archive,
        "OTTER::EVENT_MODEL",
        event_model,
        true
    );
    CHECK_OTF2_ERROR_CODE(ret);

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

    
    /* write Otter version string as 2nd entry so it is always at index 1 */
    OTF2_GlobalDefWriter_WriteString(Defs, get_unique_str_ref(), OTTER_VERSION_STRING);

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
    OTF2_GlobalDefWriter_WriteString(Defs, g_loc_grp_name, 
#if defined(OTTER_SERIAL_MODE)
    "Serial Process"
#else
    "OMP Process"
#endif
    );
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
    #include "src/otter-trace/trace-attribute-defs.h"

    /* define attributes which can be referred to later by the enum 
       attr_name_enum_t */
    #define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                \
        OTF2_GlobalDefWriter_WriteAttribute(Defs, attr_##Name,                 \
            attr_name_ref[attr_##Name][0],                                     \
            attr_name_ref[attr_##Name][1],                                     \
            Type);
    #include "src/otter-trace/trace-attribute-defs.h"

    trace_init_str_registry(
        string_registry_make(get_unique_str_ref, trace_write_string_ref)
    );

    return true;
}

bool
trace_finalise_archive(void)
{
    trace_destroy_str_registry();

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
/*  TIMESTAMP & UNIQUE REFERENCES                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static uint64_t 
get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}
