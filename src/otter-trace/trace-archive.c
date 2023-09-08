/**
 * @file trace-archive.c
 * @author Adam Tuft
 * @brief Responsible for initialising and finalising single instances of the
 * trace archive and its global definitions writer. Returns pointers to these
 * resources as well as mutexes protecting access to them both.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include <otf2/OTF2_Pthread_Locks.h>
#include <otf2/otf2.h>

#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-trace/trace-ompt.h"
#include "public/otter-version.h"

#include "trace-archive-impl.h"
#include "trace-attributes.h"
#include "trace-check-error-code.h"
#include "trace-static-constants.h"
#include "trace-timestamp.h"
#include "trace-unique-refs.h"

/* Lookup tables mapping enum value to string ref */
OTF2_StringRef attr_name_ref[n_attr_defined][2] = {0};
OTF2_StringRef attr_label_ref[n_attr_label_defined] = {0};

/* Pre- and post-flush callbacks required by OTF2 */
static OTF2_FlushType pre_flush(void *userData, OTF2_FileType fileType,
                                OTF2_LocationRef location, void *callerData,
                                bool final) {
  return OTF2_FLUSH;
}

static OTF2_TimeStamp post_flush(void *userData, OTF2_FileType fileType,
                                 OTF2_LocationRef location) {
  return get_timestamp();
}

bool trace_initialise_archive(const char *archive_path,
                              const char *archive_name,
                              otter_event_model_t event_model,
                              OTF2_Archive **archive,
                              OTF2_GlobalDefWriter **global_def_writer) {
  OTF2_ErrorCode ret = OTF2_SUCCESS;

  /* open OTF2 archive */
  OTF2_Archive *_archive =
      OTF2_Archive_Open(archive_path, /* archive path */
                        archive_name, /* archive name */
                        OTF2_FILEMODE_WRITE, OTF2_CHUNK_SIZE_EVENTS_DEFAULT,
                        OTF2_CHUNK_SIZE_DEFINITIONS_DEFAULT,
                        OTF2_SUBSTRATE_POSIX, OTF2_COMPRESSION_NONE);
  *archive = _archive;

  /* set flush callbacks */
  static OTF2_FlushCallbacks on_flush = {.otf2_pre_flush = pre_flush,
                                         .otf2_post_flush = post_flush};
  OTF2_Archive_SetFlushCallbacks(_archive, &on_flush, NULL);

  /* set serial (not MPI) collective callbacks */
  OTF2_Archive_SetSerialCollectiveCallbacks(_archive);

  /* set pthread archive locking callbacks */
  OTF2_Pthread_Archive_SetLockingCallbacks(_archive, NULL);

  /* open archive event files */
  OTF2_Archive_OpenEvtFiles(_archive);

  /* open (thread-) local definition files */
  OTF2_Archive_OpenDefFiles(_archive);

  /* get global definitions writer */
  OTF2_GlobalDefWriter *_defs = OTF2_Archive_GetGlobalDefWriter(_archive);
  *global_def_writer = _defs;

  /* detect the chosen event model and set the trace property for this */
  const char *event_model_name = NULL;
  const char *location_group_name = NULL;
  switch (event_model) {
  case otter_event_model_omp:
    event_model_name = "OMP";
    location_group_name = "OMP Process";
    break;

  case otter_event_model_serial:
    // otter-serial uses the same event model as otter-ompt
    event_model_name = "OMP";
    location_group_name = "Serial Process";
    break;

  case otter_event_model_task_graph:
    event_model_name = "TASKGRAPH";
    location_group_name = "Task-graph Process";
    break;

  default:
    event_model_name = "UNKNOWN";
    location_group_name = "Unknown Process";
    break;
  }

  ret = OTF2_Archive_SetProperty(_archive, "OTTER::EVENT_MODEL",
                                 event_model_name, true);
  CHECK_OTF2_ERROR_CODE(ret);

  /* get clock resolution & current time for CLOCK_MONOTONIC */
  struct timespec res, tp;
  if (clock_getres(CLOCK_MONOTONIC, &res) != 0) {
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
  OTF2_GlobalDefWriter_WriteClockProperties(
      _defs, 1000000000 / res.tv_nsec, /* ticks per second */
      epoch, UINT64_MAX                /* length */
  );

  /* write an empty string as the first entry so that string ref 0 is "" */
  OTF2_GlobalDefWriter_WriteString(_defs, get_unique_str_ref(), "");

  /* write Otter version string as 2nd entry so it is always at index 1 */
  OTF2_GlobalDefWriter_WriteString(_defs, get_unique_str_ref(),
                                   OTTER_VERSION_STRING);

  /* write global system tree */
  OTF2_SystemTreeNodeRef g_sys_tree_id = DEFAULT_SYSTEM_TREE;
  OTF2_StringRef g_sys_tree_name = get_unique_str_ref();
  OTF2_StringRef g_sys_tree_class = get_unique_str_ref();
  OTF2_GlobalDefWriter_WriteString(_defs, g_sys_tree_name, "Sytem Tree");
  OTF2_GlobalDefWriter_WriteString(_defs, g_sys_tree_class, "node");
  OTF2_GlobalDefWriter_WriteSystemTreeNode(_defs, g_sys_tree_id,
                                           g_sys_tree_name, g_sys_tree_class,
                                           OTF2_UNDEFINED_SYSTEM_TREE_NODE);

  /* write global location group */
  OTF2_StringRef g_loc_grp_name = get_unique_str_ref();
  OTF2_LocationGroupRef g_loc_grp_id = DEFAULT_LOCATION_GRP;
  OTF2_GlobalDefWriter_WriteString(_defs, g_loc_grp_name, location_group_name);
  OTF2_GlobalDefWriter_WriteLocationGroup(_defs, g_loc_grp_id, g_loc_grp_name,
                                          OTF2_LOCATION_GROUP_TYPE_PROCESS,
                                          g_sys_tree_id);

  /* define any necessary attributes (their names, descriptions & labels)
     these are defined in trace-attribute-defs.h and included via macros to
     reduce code repetition. */

  /* Populate lookup tables with unique string refs */
  int k = 0;
  for (k = 0; k < n_attr_defined; k++) {
    attr_name_ref[k][0] = get_unique_str_ref();
    attr_name_ref[k][1] = get_unique_str_ref();
  }

  for (k = 0; k < n_attr_label_defined; k++)
    attr_label_ref[k] = get_unique_str_ref();

/* read attributes from header and write name, description & label strings.
   lookup the string refs using the enum value for a particular attribute &
   label */
#define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                    \
  OTF2_GlobalDefWriter_WriteString(_defs, attr_name_ref[attr_##Name][0],       \
                                   #Name);                                     \
  OTF2_GlobalDefWriter_WriteString(_defs, attr_name_ref[attr_##Name][1], Desc);
#define INCLUDE_LABEL(Name, Label)                                             \
  OTF2_GlobalDefWriter_WriteString(                                            \
      _defs, attr_label_ref[attr_##Name##_##Label], #Label);
#include "trace-attribute-defs.h"

/* define attributes which can be referred to later by the enum
   attr_name_enum_t */
#define INCLUDE_ATTRIBUTE(Type, Name, Desc)                                    \
  OTF2_GlobalDefWriter_WriteAttribute(_defs, attr_##Name,                      \
                                      attr_name_ref[attr_##Name][0],           \
                                      attr_name_ref[attr_##Name][1], Type);
#include "trace-attribute-defs.h"

  return true;
}

bool trace_finalise_archive(OTF2_Archive *archive) {
  /* close event files */
  OTF2_Archive_CloseEvtFiles(archive);

  /* create 1 definition writer per location & immediately close it - not
     currently used
   */
  uint64_t nloc = get_unique_loc_ref();
  int loc = 0;
  for (loc = 0; loc < nloc; loc++) {
    OTF2_DefWriter *dw = OTF2_Archive_GetDefWriter(archive, loc);
    OTF2_Archive_CloseDefWriter(archive, dw);
  }

  /* close local definition files */
  OTF2_Archive_CloseDefFiles(archive);

  /* close OTF2 archive */
  OTF2_Archive_Close(archive);

  return true;
}

/**
 * @brief Write a string definition to a trace.
 *
 * @param s The string to write
 * @param ref The ref of the string
 * @param def_writer must be a valid `OTF2_GlobalDefWriter*`
 */
void trace_archive_write_string_ref(OTF2_GlobalDefWriter *def_writer,
                                    OTF2_StringRef ref, const char *s) {
  if (def_writer == NULL) {
    LOG_ERROR("def_writer was null, unable to write ref for string: \"%s\"", s);
    return;
  }
  LOG_DEBUG("writing ref %u for string \"%s\"", ref, s);
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_GlobalDefWriter_WriteString(def_writer, ref, s);
  CHECK_OTF2_ERROR_CODE(r);
}
