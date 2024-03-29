#include "public/otter-trace/trace-region-def.h"
#include "public/otter-trace/trace-ompt.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include <assert.h>
#include <otf2/OTF2_Definitions.h>
#include <pthread.h>
#include <stdlib.h>

#include "trace-archive-impl.h"
#include "trace-attribute-lookup.h"
#include "trace-attributes.h"
#include "trace-check-error-code.h"
#include "trace-state.h"
#include "trace-static-constants.h"
#include "trace-types-as-labels.h"
#include "trace-unique-refs.h"

/* Store values needed to register region definition (tasks, parallel regions,
   workshare constructs etc.) with OTF2 */
typedef struct trace_region_def_t {
  OTF2_RegionRef ref;
  OTF2_RegionRole role;
  trace_region_type_t type;
  unique_id_t encountering_task_id;
  otter_stack_t *rgn_stack;
  trace_region_attr_t attr;
} trace_region_def_t;

// Constructors

trace_region_def_t *trace_new_master_region(unique_id_t thread_id,
                                            unique_id_t encountering_task_id) {
  trace_region_def_t *new = malloc(sizeof(*new));
  *new = (trace_region_def_t){.ref = get_unique_rgn_ref(),
                              .role = OTF2_REGION_ROLE_MASTER,
                              .type = trace_region_master,
                              .encountering_task_id = encountering_task_id,
                              .rgn_stack = NULL,
                              .attr.master = {.thread = thread_id}};
  return new;
}

trace_region_def_t *
trace_new_parallel_region(unique_id_t id, unique_id_t master,
                          unique_id_t encountering_task_id, int flags,
                          unsigned int requested_parallelism) {
  trace_region_def_t *new = malloc(sizeof(*new));
  *new = (trace_region_def_t){
      .ref = get_unique_rgn_ref(),
      .role = OTF2_REGION_ROLE_PARALLEL,
      .type = trace_region_parallel,
      .encountering_task_id = encountering_task_id,
      .rgn_stack = NULL,
      .attr.parallel = {.id = id,
                        .master_thread = master,
                        .is_league =
                            flags & otter_parallel_league ? true : false,
                        .requested_parallelism = requested_parallelism,
                        .ref_count = 0,
                        .enter_count = 0,
                        .lock_rgn = PTHREAD_MUTEX_INITIALIZER,
                        .rgn_defs = queue_create()}};
  return new;
}

trace_region_def_t *trace_new_phase_region(otter_phase_region_t type,
                                           unique_id_t encountering_task_id,
                                           const char *phase_name) {
  trace_region_def_t *new = malloc(sizeof(*new));
  *new = (trace_region_def_t){.ref = get_unique_rgn_ref(),
                              .role = OTF2_REGION_ROLE_CODE,
                              .type = trace_region_phase,
                              .encountering_task_id = encountering_task_id,
                              .rgn_stack = NULL,
                              .attr.phase = {.type = type, .name = 0}};

  if (phase_name != NULL) {
    pthread_mutex_lock(&state.strings.lock);
    new->attr.phase.name =
        string_registry_insert(state.strings.instance, phase_name);
    pthread_mutex_unlock(&state.strings.lock);
  } else {
    new->attr.phase.name = 0;
  }
  return new;
}

trace_region_def_t *trace_new_sync_region(otter_sync_region_t stype,
                                          trace_task_sync_t task_sync_mode,
                                          unique_id_t encountering_task_id) {
  trace_region_def_t *new = malloc(sizeof(*new));
  OTF2_RegionRole role = OTF2_REGION_ROLE_UNKNOWN;
  switch (stype) {
  case otter_sync_region_barrier:
    role = OTF2_REGION_ROLE_BARRIER;
    break;
  case otter_sync_region_barrier_implicit:
    role = OTF2_REGION_ROLE_IMPLICIT_BARRIER;
    break;
  case otter_sync_region_barrier_explicit:
    role = OTF2_REGION_ROLE_BARRIER;
    break;
  case otter_sync_region_barrier_implementation:
    role = OTF2_REGION_ROLE_BARRIER;
    break;
  case otter_sync_region_taskwait:
    role = OTF2_REGION_ROLE_TASK_WAIT;
    break;
  case otter_sync_region_taskgroup:
    role = OTF2_REGION_ROLE_TASK_WAIT;
    break;
  default:
    break;
  }
  *new = (trace_region_def_t){
      .ref = get_unique_rgn_ref(),
      .role = role,
      .type = trace_region_synchronise,
      .encountering_task_id = encountering_task_id,
      .rgn_stack = NULL,
      .attr.sync = {
          .type = stype,
          .sync_descendant_tasks =
              (task_sync_mode == trace_sync_descendants ? true : false)}};
  return new;
}

trace_region_def_t *
trace_new_task_region(trace_region_def_t *parent_task_region, unique_id_t id,
                      otter_task_flag_t flags, int has_dependences,
                      otter_src_location_t *src_location,
                      const void *task_create_ra) {
  /* Create a region representing a task. Add to the location's region
     definition queue. */

  /* A task maintains a stack of the active regions encountered during its
     execution up to a task-switch event, which is restored to the executing
     thread when the task is resumed */

  LOG_DEBUG_IF((src_location), "got src_location(file=%s, func=%s, line=%d)",
               src_location->file, src_location->func, src_location->line);

  trace_region_def_t *new = malloc(sizeof(*new));
  *new = (trace_region_def_t){
      .ref = get_unique_rgn_ref(),
      .role = OTF2_REGION_ROLE_TASK,
      .type = trace_region_task,
      .rgn_stack = stack_create(),
      .attr.task = {
          .id = id,
          .type = flags & otter_task_type_mask,
          .flags = flags,
          .has_dependences = has_dependences,
          .parent_id = parent_task_region != NULL
                           ? parent_task_region->attr.task.id
                           : OTF2_UNDEFINED_UINT64,
          .parent_type = parent_task_region != NULL
                             ? parent_task_region->attr.task.type
                             : OTF2_UNDEFINED_UINT32,
          .task_status = otter_task_state_undef,
          .source_file_name_ref = 0,
          .source_func_name_ref = 0,
          .source_line_number = 0,
      }};
  new->encountering_task_id = new->attr.task.parent_id;

  if (src_location != NULL) {
    pthread_mutex_lock(&state.strings.lock);
    new->attr.task.source_file_name_ref =
        string_registry_insert(state.strings.instance, src_location->file);
    new->attr.task.source_func_name_ref =
        string_registry_insert(state.strings.instance, src_location->func);
    pthread_mutex_unlock(&state.strings.lock);
    new->attr.task.source_line_number = src_location->line;
  } else {
    new->attr.task.source_file_name_ref = 0;
    new->attr.task.source_func_name_ref = 0;
    new->attr.task.source_line_number = 0;
  }
  return new;
}

trace_region_def_t *
trace_new_workshare_region(otter_work_t wstype, uint64_t count,
                           unique_id_t encountering_task_id) {
  trace_region_def_t *new = malloc(sizeof(*new));
  OTF2_RegionRole role = OTF2_REGION_ROLE_UNKNOWN;
  switch (wstype) {
  case otter_work_loop:
    role = OTF2_REGION_ROLE_LOOP;
    break;
  case otter_work_sections:
    role = OTF2_REGION_ROLE_SECTIONS;
    break;
  case otter_work_single_executor:
    role = OTF2_REGION_ROLE_SINGLE;
    break;
  case otter_work_single_other:
    role = OTF2_REGION_ROLE_SINGLE;
    break;
  case otter_work_workshare:
    role = OTF2_REGION_ROLE_WORKSHARE;
    break;
  case otter_work_distribute:
    role = OTF2_REGION_ROLE_UNKNOWN;
    break;
  case otter_work_taskloop:
    role = OTF2_REGION_ROLE_LOOP;
    break;
  default:
    break;
  }
  *new = (trace_region_def_t){.ref = get_unique_rgn_ref(),
                              .role = role,
                              .type = trace_region_workshare,
                              .encountering_task_id = encountering_task_id,
                              .rgn_stack = NULL,
                              .attr.wshare = {.type = wstype, .count = count}};
  return new;
}

// Destructors

void trace_destroy_master_region(trace_region_def_t *rgn) {
  LOG_DEBUG("region %p", rgn);
  free(rgn);
}

void trace_destroy_parallel_region(trace_region_def_t *rgn) {
  if (rgn->type != trace_region_parallel) {
    LOG_ERROR("invalid region type %d", rgn->type);
    abort();
  }

  size_t n_defs = queue_length(rgn->attr.parallel.rgn_defs);
  LOG_DEBUG("[parallel=%lu] writing nested region definitions (%lu)",
            rgn->attr.parallel.id, n_defs);

  /* Write parallel region's definition */
  trace_region_write_definition(rgn);

  /* write region's nested region definitions */
  trace_region_def_t *r = NULL;
  int count = 0;
  while (queue_pop(rgn->attr.parallel.rgn_defs, (data_item_t *)&r)) {
    LOG_DEBUG("[parallel=%lu] writing region definition %d/%lu (region %3u)",
              rgn->attr.parallel.id, count + 1, n_defs, r->ref);
    count++;
    trace_region_write_definition(r);

    /* destroy each region once its definition is written */
    switch (r->type) {
    case trace_region_workshare:
      trace_destroy_workshare_region(r);
      break;

    case trace_region_master:
      trace_destroy_master_region(r);
      break;

    case trace_region_synchronise:
      trace_destroy_sync_region(r);
      break;

    case trace_region_task:
      trace_destroy_task_region(r);
      break;

    case trace_region_phase:
      trace_destroy_phase_region(r);
      break;

    default:
      LOG_ERROR("unknown region type %d", r->type);
      abort();
    }
  }

  /* destroy parallel region once all locations are done with it
     and all definitions written */
  queue_destroy(rgn->attr.parallel.rgn_defs, false, NULL);
  LOG_DEBUG("region %p (parallel id %lu)", rgn, rgn->attr.parallel.id);
  free(rgn);
  return;
}

void trace_destroy_phase_region(trace_region_def_t *rgn) {
  LOG_DEBUG("region %p", rgn);
  free(rgn);
}

void trace_destroy_sync_region(trace_region_def_t *rgn) {
  LOG_DEBUG("region %p", rgn);
  free(rgn);
}

void trace_destroy_task_region(trace_region_def_t *rgn) {
  LOG_WARN_IF((!(rgn->attr.task.task_status == otter_task_complete ||
                 rgn->attr.task.task_status == otter_task_cancel)),
              "destroying task region before task-complete/task-cancel");
  LOG_DEBUG("region %p destroying active regions stack %p", rgn,
            rgn->rgn_stack);
  stack_destroy(rgn->rgn_stack, false, NULL);
  LOG_DEBUG("region %p", rgn);
  free(rgn);
}

void trace_destroy_workshare_region(trace_region_def_t *rgn) {
  LOG_DEBUG("region %p", rgn);
  free(rgn);
}

// Add attributes

void trace_add_region_type_attributes(trace_region_def_t *rgn,
                                      OTF2_AttributeList *attributes) {
  switch (rgn->type) {
  case trace_region_parallel:
    trace_add_parallel_attributes(rgn, attributes);
    break;
  case trace_region_workshare:
    trace_add_workshare_attributes(rgn, attributes);
    break;
  case trace_region_synchronise:
    trace_add_sync_attributes(rgn, attributes);
    break;
  case trace_region_task:
    trace_add_task_attributes(rgn, attributes);
    break;
  case trace_region_master:
    trace_add_master_attributes(rgn, attributes);
    break;
  case trace_region_phase:
    trace_add_phase_attributes(rgn, attributes);
    break;
  }
}

void trace_add_master_attributes(trace_region_def_t *rgn,
                                 OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddUint64(attributes, attr_unique_id,
                                   rgn->attr.master.thread);
  CHECK_OTF2_ERROR_CODE(r);
  return;
}

void trace_add_parallel_attributes(trace_region_def_t *rgn,
                                   OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddUint64(attributes, attr_unique_id,
                                   rgn->attr.parallel.id);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint32(attributes, attr_requested_parallelism,
                                   rgn->attr.parallel.requested_parallelism);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddStringRef(attributes, attr_is_league,
                                      rgn->attr.parallel.is_league
                                          ? attr_label_ref[attr_flag_true]
                                          : attr_label_ref[attr_flag_false]);
  CHECK_OTF2_ERROR_CODE(r);
  return;
}

void trace_add_phase_attributes(trace_region_def_t *rgn,
                                OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_phase_type,
      attr_label_ref[attr_region_type_generic_phase]);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddStringRef(attributes, attr_phase_name,
                                      rgn->attr.phase.name);
  CHECK_OTF2_ERROR_CODE(r);
  return;
}

void trace_add_sync_attributes(trace_region_def_t *rgn,
                               OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_sync_type,
      attr_label_ref[sync_type_as_label(rgn->attr.sync.type)]);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(
      attributes, attr_sync_descendant_tasks,
      (uint8_t)(rgn->attr.sync.sync_descendant_tasks ? 1 : 0));
  return;
}

void trace_add_task_attributes(trace_region_def_t *rgn,
                               OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddUint64(attributes, attr_unique_id,
                                   rgn->attr.task.id);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_task_type,
      attr_label_ref[task_type_as_label(rgn->attr.task.type)]);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint32(attributes, attr_task_flags,
                                   rgn->attr.task.flags);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint64(attributes, attr_parent_task_id,
                                   rgn->attr.task.parent_id);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_parent_task_type,
      attr_label_ref[task_type_as_label(rgn->attr.task.parent_type)]);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_has_dependences,
                                  rgn->attr.task.has_dependences);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_is_undeferred,
                                  rgn->attr.task.flags & otter_task_undeferred);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_is_untied,
                                  rgn->attr.task.flags & otter_task_untied);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_is_final,
                                  rgn->attr.task.flags & otter_task_final);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_is_mergeable,
                                  rgn->attr.task.flags & otter_task_mergeable);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint8(attributes, attr_task_is_merged,
                                  rgn->attr.task.flags & otter_task_merged);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_prior_task_status,
      attr_label_ref[task_status_as_label(rgn->attr.task.task_status)]);
  CHECK_OTF2_ERROR_CODE(r);

  // Add source location if defined for this task
  if (rgn->attr.task.source_file_name_ref != 0) {
    r = OTF2_AttributeList_AddUint32(attributes, attr_source_line_number,
                                     rgn->attr.task.source_line_number);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(attributes, attr_source_file_name,
                                        rgn->attr.task.source_file_name_ref);
    CHECK_OTF2_ERROR_CODE(r);
    r = OTF2_AttributeList_AddStringRef(attributes, attr_source_func_name,
                                        rgn->attr.task.source_func_name_ref);
    CHECK_OTF2_ERROR_CODE(r);
  }
  return;
}

void trace_add_workshare_attributes(trace_region_def_t *rgn,
                                    OTF2_AttributeList *attributes) {
  OTF2_ErrorCode r = OTF2_SUCCESS;
  r = OTF2_AttributeList_AddStringRef(
      attributes, attr_workshare_type,
      attr_label_ref[work_type_as_label(rgn->attr.wshare.type)]);
  CHECK_OTF2_ERROR_CODE(r);
  r = OTF2_AttributeList_AddUint64(attributes, attr_workshare_count,
                                   rgn->attr.wshare.count);
  CHECK_OTF2_ERROR_CODE(r);
  return;
}

// Getters

OTF2_RegionRef trace_region_get_ref(trace_region_def_t *region) {
  return region->ref;
}

unique_id_t trace_region_get_encountering_task_id(trace_region_def_t *region) {
  return region->encountering_task_id;
}

trace_region_attr_t trace_region_get_attributes(trace_region_def_t *region) {
  return region->attr;
}

trace_region_type_t trace_region_get_type(trace_region_def_t *region) {
  return region->type;
}

otter_queue_t *trace_region_get_rgn_def_queue(trace_region_def_t *region) {
  // This operation is only valid for parallel regions
  assert(region->type == trace_region_parallel);
  return region->attr.parallel.rgn_defs;
}

otter_stack_t *trace_region_get_task_rgn_stack(trace_region_def_t *region) {
  // This operation is only valid for task regions
  assert(region->type == trace_region_task);
  return region->rgn_stack;
}

unsigned int trace_region_get_shared_ref_count(trace_region_def_t *region) {
  assert(trace_region_is_shared(region));
  return region->attr.parallel.ref_count;
}

// Setters

void trace_region_set_task_status(trace_region_def_t *region,
                                  otter_task_status_t status) {
  assert(region->type == trace_region_task);
  region->attr.task.task_status = status;
}

// Lock and unlock shared regions

bool trace_region_is_type(trace_region_def_t *region,
                          trace_region_type_t region_type) {
  return region->type == region_type;
}

bool trace_region_is_shared(trace_region_def_t *region) {
  return region->type == trace_region_parallel;
}

void trace_region_lock(trace_region_def_t *region) {
  assert(trace_region_is_shared(region));
  pthread_mutex_lock(&region->attr.parallel.lock_rgn);
}

void trace_region_unlock(trace_region_def_t *region) {
  assert(trace_region_is_shared(region));
  pthread_mutex_unlock(&region->attr.parallel.lock_rgn);
}

void trace_region_inc_ref_count(trace_region_def_t *region) {
  assert(trace_region_is_shared(region));
  region->attr.parallel.ref_count++;
  region->attr.parallel.enter_count++;
}

void trace_region_dec_ref_count(trace_region_def_t *region) {
  assert(trace_region_is_shared(region));
  region->attr.parallel.ref_count--;
}

// Write region definition to a trace
void trace_region_write_definition(trace_region_def_t *region) {
  if (region == NULL) {
    LOG_ERROR("null pointer");
    return;
  }

  LOG_DEBUG("writing region definition %3u (type=%3d, role=%3u) %p",
            region->ref, region->type, region->role, region);

  pthread_mutex_lock(&state.global_def_writer.lock);
  OTF2_GlobalDefWriter *writer = state.global_def_writer.instance;

  switch (region->type) {
  case trace_region_parallel: {
    char region_name[default_name_buf_sz + 1] = {0};
    snprintf(region_name, default_name_buf_sz, "Parallel Region %lu",
             region->attr.parallel.id);
    OTF2_StringRef region_name_ref = get_unique_str_ref();
    OTF2_GlobalDefWriter_WriteString(writer, region_name_ref, region_name);
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref, region_name_ref, 0,
        0, /* canonical name, description */
        region->role, OTF2_PARADIGM_UNKNOWN, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  case trace_region_workshare: {
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref,
        attr_label_ref[work_type_as_label(region->attr.wshare.type)], 0, 0,
        region->role, OTF2_PARADIGM_UNKNOWN, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  case trace_region_master: {
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref, attr_label_ref[attr_region_type_master], 0, 0,
        region->role, OTF2_PARADIGM_UNKNOWN, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  case trace_region_synchronise: {
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref,
        attr_label_ref[sync_type_as_label(region->attr.sync.type)], 0, 0,
        region->role, OTF2_PARADIGM_UNKNOWN, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  case trace_region_task: {
    char task_name[default_name_buf_sz + 1] = {0};
    snprintf(task_name, default_name_buf_sz, "%s task %lu",
             region->attr.task.type == otter_task_initial    ? "initial"
             : region->attr.task.type == otter_task_implicit ? "implicit"
             : region->attr.task.type == otter_task_explicit ? "explicit"
             : region->attr.task.type == otter_task_target   ? "target"
                                                             : "??",
             region->attr.task.id);
    OTF2_StringRef task_name_ref = get_unique_str_ref();
    OTF2_GlobalDefWriter_WriteString(writer, task_name_ref, task_name);
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref, task_name_ref, 0,
        0, /* canonical name, description */
        region->role, OTF2_PARADIGM_OPENMP, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  case trace_region_phase: {
    OTF2_GlobalDefWriter_WriteRegion(
        writer, region->ref, attr_label_ref[attr_region_type_generic_phase], 0,
        0, region->role, OTF2_PARADIGM_UNKNOWN, OTF2_REGION_FLAG_NONE, 0, 0,
        0); /* source file, begin line no., end line no. */
    break;
  }
  default: {
    LOG_ERROR("unexpected region type %d", region->type);
  }
  }
  pthread_mutex_unlock(&state.global_def_writer.lock);
  return;
}
