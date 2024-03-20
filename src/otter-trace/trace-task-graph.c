#define _GNU_SOURCE
#define USE_LOCAL_EVENT_WRITER

/**
 * TODO:
 *
 *  - manage source locations somewhere - map file & line to
 * OTF2_SourceCodeLocationRef
 *  - use OTF2_GlobalDefWriter_WriteSourceCodeLocation to write source locations
 */

#include <execinfo.h>
#include <otf2/otf2.h>
#include <pthread.h>
#include <time.h>

#include "public/debug.h"
#include "public/otter-common.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/trace-task-context-interface.h"
#include "public/otter-trace/trace-task-graph.h"
#include "public/otter-trace/trace-thread-data.h"
#include "public/types/queue.h"

#include "trace-archive-impl.h"
#include "trace-attribute-lookup.h"
#include "trace-attributes.h"
#include "trace-check-error-code.h"
#include "trace-state.h"
#include "trace-timestamp.h"
#include "trace-types-as-labels.h"
#include "trace-unique-refs.h"

#define BACKTRACE_DEPTH 10

enum { void_ptr_top_6_bytes_mask = 0xffffffffffff0000 };

static inline void *get_user_code_return_address(void) {
  /**
   * @brief Get the apparent return address of the code which called into
   * Otter - the heuristic is that we look for an address which appears to
   * come from a different memory-mapped region than Otter appears to occupy.
   * Detect this by comparing the top bits of addresses in a backtrace for the
   * first address with different bits.
   *
   * Note that this depends on Otter being built as a shared library and
   * would stop working if Otter was linked statically
   */
  void *ra_buffer[BACKTRACE_DEPTH] = {NULL};
  int bt_size = backtrace(&ra_buffer[0], BACKTRACE_DEPTH);
  void *this_frame_top_bits =
      (void *)(((unsigned long)ra_buffer[1]) & void_ptr_top_6_bytes_mask);
  LOG_DEBUG("backtrace:");
  for (int j = 0; j < bt_size; j++) {
    void *address = ra_buffer[j];
    void *top_bits =
        (void *)(((unsigned long)address) & void_ptr_top_6_bytes_mask);
    if (top_bits != this_frame_top_bits) {
      LOG_DEBUG("    backtrace[%2d] %p with top bits %p <---", j, address,
                top_bits);
      return address;
    } else {
      LOG_DEBUG("    backtrace[%2d] %p with top bits %p", j, address, top_bits);
    }
  }
  return NULL;
}

void trace_graph_event_task_create(trace_location_def_t *location,
                                   unique_id_t encountering_task_id,
                                   unique_id_t new_task_id,
                                   otter_string_ref_t task_label,
                                   otter_src_ref_t create_ref) {
  LOG_DEBUG("record task-graph event: task create");

  OTF2_ErrorCode err = OTF2_SUCCESS;
  OTF2_AttributeList *attr = OTF2_AttributeList_New();
  OTF2_EvtWriter *event_writer = NULL;

  trace_location_get_otf2(location, NULL, &event_writer, NULL);

  err = OTF2_AttributeList_AddUint64(attr, attr_encountering_task_id,
                                     encountering_task_id);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddUint64(attr, attr_unique_id, new_task_id);
  CHECK_OTF2_ERROR_CODE(err);

  err =
      OTF2_AttributeList_AddStringRef(attr, attr_source_file, create_ref.file);
  CHECK_OTF2_ERROR_CODE(err);

  err =
      OTF2_AttributeList_AddStringRef(attr, attr_source_func, create_ref.func);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddInt32(attr, attr_source_line, create_ref.line);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_endpoint,
                                        attr_label_ref[attr_endpoint_discrete]);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_task_label, task_label);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(
      attr, attr_event_type, attr_label_ref[attr_event_type_task_create]);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_EvtWriter_ThreadTaskCreate(event_writer, attr, get_timestamp(),
                                        OTF2_UNDEFINED_COMM,
                                        OTF2_UNDEFINED_UINT32, 0);
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_AttributeList_Delete(attr);
}

/**
 * @brief Record a task-enter event with these attributes:
 *  - encountering task (the task entered)
 *  - event type i.e. task-enter
 *  - endpoint i.e. enter
 *  - source location
 *
 * @param location
 * @param encountering_task_id
 * @param start_ref
 */
void trace_graph_event_task_begin(trace_location_def_t *location,
                                  unique_id_t encountering_task_id,
                                  otter_src_ref_t start_ref) {
  LOG_DEBUG("record task-graph event: task begin");

  OTF2_ErrorCode err = OTF2_SUCCESS;
  OTF2_AttributeList *attr = OTF2_AttributeList_New();
  OTF2_EvtWriter *event_writer = NULL;

  trace_location_get_otf2(location, NULL, &event_writer, NULL);

  err = OTF2_AttributeList_AddUint64(attr, attr_encountering_task_id,
                                     encountering_task_id);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(
      attr, attr_event_type, attr_label_ref[attr_event_type_task_enter]);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_endpoint,
                                        attr_label_ref[attr_endpoint_enter]);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_file, start_ref.file);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_func, start_ref.func);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddInt32(attr, attr_source_line, start_ref.line);
  CHECK_OTF2_ERROR_CODE(err);

  // Record event
  err = OTF2_EvtWriter_ThreadTaskSwitch(
      event_writer, attr, get_timestamp(), OTF2_UNDEFINED_COMM,
      OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_AttributeList_Delete(attr);
}

/**
 * @brief Record a task-complete event with these attributes:
 *  - encountering task (the task completed)
 *  - event type i.e. task-complete
 *  - endpoint i.e. leave
 *  - source location
 *
 * @param location
 * @param encountering_task_id
 * @param end_ref
 */
void trace_graph_event_task_end(trace_location_def_t *location,
                                unique_id_t encountering_task_id,
                                otter_src_ref_t end_ref) {
  LOG_DEBUG("record task-graph event: task leave");

  OTF2_ErrorCode err = OTF2_SUCCESS;
  OTF2_AttributeList *attr = OTF2_AttributeList_New();
  OTF2_EvtWriter *event_writer = NULL;

  trace_location_get_otf2(location, NULL, &event_writer, NULL);

  err = OTF2_AttributeList_AddUint64(attr, attr_encountering_task_id,
                                     encountering_task_id);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(
      attr, attr_event_type, attr_label_ref[attr_event_type_task_leave]);
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_AttributeList_AddStringRef(attr, attr_endpoint,
                                  attr_label_ref[attr_endpoint_leave]);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_file, end_ref.file);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_func, end_ref.func);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddInt32(attr, attr_source_line, end_ref.line);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_EvtWriter_ThreadTaskSwitch(
      event_writer, attr, get_timestamp(), OTF2_UNDEFINED_COMM,
      OTF2_UNDEFINED_UINT32, 0); /* creating thread, generation number */
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_AttributeList_Delete(attr);
}

/**
 * @brief Record a task-sync event with these attributes:
 *  - encountering task (the task which blocks on its dependencies)
 *  - region type (i.e. taskwait)
 *  - event type i.e. task-sync
 *  - endpoint i.e. enter/leave
 *  - sync mode (i.e. children or descendants)
 *
 */
void trace_graph_synchronise_tasks(trace_location_def_t *location,
                                   unique_id_t encountering_task_id,
                                   trace_sync_region_attr_t sync_attr,
                                   otter_endpoint_t endpoint,
                                   otter_src_ref_t src_ref) {
  LOG_DEBUG("record task-graph event: synchronise");

  OTF2_ErrorCode err = OTF2_SUCCESS;
  OTF2_AttributeList *attr = OTF2_AttributeList_New();
  OTF2_EvtWriter *event_writer = NULL;

  trace_location_get_otf2(location, NULL, &event_writer, NULL);

  err = OTF2_AttributeList_AddUint64(attr, attr_encountering_task_id,
                                     encountering_task_id);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(
      attr, attr_region_type,
      attr_label_ref[sync_type_as_label(sync_attr.type)]);
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_StringRef event_type, endpoint_str;
  switch (endpoint) {
  case otter_endpoint_enter:
    event_type = attr_label_ref[attr_event_type_sync_begin];
    endpoint_str = attr_label_ref[attr_endpoint_enter];
    break;
  case otter_endpoint_leave:
    event_type = attr_label_ref[attr_event_type_sync_end];
    endpoint_str = attr_label_ref[attr_endpoint_leave];
    break;
  case otter_endpoint_discrete:
    event_type = attr_label_ref[attr_event_type_sync_begin];
    endpoint_str = attr_label_ref[attr_endpoint_discrete];
    break;
  }

  err = OTF2_AttributeList_AddStringRef(attr, attr_event_type, event_type);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_endpoint, endpoint_str);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddUint8(attr, attr_sync_descendant_tasks,
                                    sync_attr.sync_descendant_tasks ? 1 : 0);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_file, src_ref.file);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddStringRef(attr, attr_source_func, src_ref.func);
  CHECK_OTF2_ERROR_CODE(err);

  err = OTF2_AttributeList_AddInt32(attr, attr_source_line, src_ref.line);
  CHECK_OTF2_ERROR_CODE(err);

  switch (endpoint) {
  case otter_endpoint_enter:
  case otter_endpoint_discrete:
    err = OTF2_EvtWriter_Enter(event_writer, attr, get_timestamp(),
                               OTF2_UNDEFINED_REGION);
    break;
  case otter_endpoint_leave:
    err = OTF2_EvtWriter_Leave(event_writer, attr, get_timestamp(),
                               OTF2_UNDEFINED_REGION);
    break;
  }
  CHECK_OTF2_ERROR_CODE(err);

  OTF2_AttributeList_Delete(attr);
}

void trace_task_graph_finalise(void) {
  LOG_DEBUG("=== Finalising trace-task-graph ===");
}
