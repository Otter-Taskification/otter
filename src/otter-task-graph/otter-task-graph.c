/**
 * @file otter-task-graph.c
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Implementation of Otter task graph event source API for recording task
 * graph via annotations
 * @version 0.2.0
 * @date 2022-10-03
 *
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 *
 */

#define OTTER_USE_PHASES 1

#define __USE_POSIX // HOST_NAME_MAX
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

#include "api/otter-task-graph/otter-task-graph.h"
#include "public/config.h"
#include "public/debug.h"
#include "public/otter-environment-variables.h"
#include "public/otter-trace/source-location.h"
#include "public/otter-trace/strings.h"
#include "public/otter-trace/trace-initialise.h"
#include "public/otter-trace/trace-task-context-interface.h"
#include "public/otter-trace/trace-task-graph.h"
#include "public/otter-trace/trace-task-manager.h"
#include "public/otter-trace/trace-thread-data.h"
#include "public/otter-version.h"
#include "public/types/queue.h"

#define LABEL_BUFFER_MAX_CHARS 256

struct thread_data_queue {
  otter_queue_t *instance;
  pthread_mutex_t lock;
};

static trace_task_manager_callback debug_print_count;
static trace_task_manager_callback debug_store_count_in_queue;

/* detect environment variables */
static otter_opt_t opt = {.hostname = NULL,
                          .tracename = NULL,
                          .tracepath = NULL,
                          .archive_name = NULL,
                          .append_hostname = false};

// The implicit root task
static otter_task_context *root_task = NULL;
static otter_task_context *phase_task = NULL;

// TODO: move into trace_state_t
static pthread_mutex_t task_manager_mutex = PTHREAD_MUTEX_INITIALIZER;
static trace_task_manager_t *task_manager = NULL;
#define TASK_MANAGER_LOCK() pthread_mutex_lock(&task_manager_mutex)
#define TASK_MANAGER_UNLOCK() pthread_mutex_unlock(&task_manager_mutex)

// per-thread state
static thread_local thread_data_t *thread_data = NULL;

// store per-thread state for clean-up at finalisation
static struct thread_data_queue thread_queue = {
    .instance = NULL, .lock = PTHREAD_MUTEX_INITIALIZER};

static inline thread_data_t *get_thread_data(void) {
  if (thread_data == NULL) {
    thread_data = new_thread_data(otter_thread_worker);
    LOG_DEBUG("allocate thread-local data for thread %" PRIu64,
              thread_data->id);
    // add to shared queue for later clean-up

    pthread_mutex_lock(&thread_queue.lock);
    if (thread_queue.instance == NULL) {
      thread_queue.instance = queue_create();
    }
    queue_push(thread_queue.instance, (data_item_t){.ptr = thread_data});
    pthread_mutex_unlock(&thread_queue.lock);
  }
  return thread_data;
}

static void otter_register_task_label_va_list(otter_task_context *task,
                                              bool add_to_task_manager,
                                              const char *format,
                                              va_list args) {
  char label_buffer[LABEL_BUFFER_MAX_CHARS] = {0};
  int chars_required =
      vsnprintf(&label_buffer[0], LABEL_BUFFER_MAX_CHARS, format, args);
  if (chars_required >= LABEL_BUFFER_MAX_CHARS) {
    LOG_WARN("label truncated (%d/%d chars written): %s",
             LABEL_BUFFER_MAX_CHARS, chars_required, label_buffer);
  }
  if (add_to_task_manager) {
    LOG_DEBUG("register task with label: %s", label_buffer);
    TASK_MANAGER_LOCK();
    trace_task_manager_add_task(task_manager, &label_buffer[0], task);
    TASK_MANAGER_UNLOCK();
  }
  otter_string_ref_t task_label_ref = get_string_ref(&label_buffer[0]);
  otterTaskContext_set_task_label_ref(task, task_label_ref);
}

void otterTraceInitialise(otter_source_args source_location) {
  // Initialise archive

  static char host[HOST_NAME_MAX + 1] = {0};
  gethostname(host, HOST_NAME_MAX);

  opt.hostname = host;
  opt.tracename = getenv(ENV_VAR_TRACE_OUTPUT);
  opt.tracepath = getenv(ENV_VAR_TRACE_PATH);
  opt.append_hostname = getenv(ENV_VAR_APPEND_HOST) == NULL ? false : true;
  opt.event_model = otter_event_model_task_graph;

  /* Apply defaults if variables not provided */
  if (opt.tracename == NULL)
    opt.tracename = DEFAULT_OTF2_TRACE_OUTPUT;
  if (opt.tracepath == NULL)
    opt.tracepath = DEFAULT_OTF2_TRACE_PATH;

  LOG_INFO("Otter environment variables:");
  LOG_INFO("%-30s %s", "host", opt.hostname);
  LOG_INFO("%-30s %s", ENV_VAR_TRACE_PATH, opt.tracepath);
  LOG_INFO("%-30s %s", ENV_VAR_TRACE_OUTPUT, opt.tracename);
  LOG_INFO("%-30s %s", ENV_VAR_APPEND_HOST, opt.append_hostname ? "Yes" : "No");

  trace_initialise(&opt);
  task_manager = trace_task_manager_alloc();

  // Write the definition of a dummy location
  // trace_write_location_definition(...)? or simply via
  // OTF2_GlobalDefWriter_WriteLocation(...)

  // Define the implicit root task
  // TODO: want to be able to set additional task attributes here e.g. task type
  root_task = otterTaskInitialise(NULL, 0, otter_no_add_to_pool,
                                  source_location, "OTTER ROOT TASK (%s:%d)",
                                  source_location.func, source_location.line);
  otterTaskStart(root_task, source_location);

  return;
}

void otterTraceFinalise(otter_source_args source) {
  // Finalise arhchive
  LOG_DEBUG("=== finalising archive ===");

  if (phase_task != NULL) {
    otterPhaseEnd(source);
  }

  // TODO: add implicit synchronisation for root_task here.

  otterTaskEnd(root_task, source);

#if DEBUG_LEVEL >= 3
  otter_queue_t *queue = queue_create();
  trace_task_manager_count_insertions(task_manager, debug_store_count_in_queue,
                                      (void *)queue);
  const char *label = NULL;
  uint64_t count = 0;
  while (queue_pop(queue, (data_item_t *)&label)) {
    queue_pop(queue, (data_item_t *)&count);
    printf("label \"%s\" had %lu insertions\n", label, count);
  }
  queue_destroy(queue, false, NULL);
#endif

  trace_task_manager_free(task_manager);

  // destroy any accumulated thread data
  void *thread_data = NULL;
  while (queue_pop(thread_queue.instance, (data_item_t *)&thread_data)) {
    LOG_DEBUG("destroy thread data %p", thread_data);
    thread_destroy(thread_data);
  }
  queue_destroy(thread_queue.instance, false, NULL);

  trace_task_graph_finalise();
  trace_finalise();

  char trace_folder[PATH_MAX] = {0};
  realpath(opt.tracepath, &trace_folder[0]);
  fprintf(stderr, "%s%s/%s\n", "OTTER_TRACE_FOLDER:", trace_folder,
          opt.archive_name);
  return;
}

otter_task_context *otterTaskInitialise(otter_task_context *parent, int flavour,
                                        otter_add_to_pool_t add_to_pool,
                                        otter_source_args init,
                                        const char *format, ...) {
  LOG_DEBUG("%s:%d in %s", init.file, init.line, init.func);
  otter_task_context *task = otterTaskContext_alloc();
  otter_src_ref_t init_ref = get_source_location_ref((otter_src_location_t){
      .file = init.file, .func = init.func, .line = init.line});

  // If no parent given, set the current phase (or root) task as the parent.
  // Only the implicit root task may have a NULL parent.
  if (parent == NULL) {
    if (phase_task != NULL) {
      parent = phase_task;
    } else if (root_task != NULL) {
      parent = root_task;
    } else {
      parent = NULL;
    }
  }

  otterTaskContext_init(task, parent, flavour, init_ref);
  va_list args;
  va_start(args, format);
  otter_register_task_label_va_list(
      task, add_to_pool == otter_add_to_pool ? true : false, format, args);
  va_end(args);
  return task;
}

otter_task_context *otterTaskStart(otter_task_context *task,
                                   otter_source_args start) {
  if (task == NULL) {
    LOG_ERROR("IGNORED (tried to start null task at %s:%d in %s)", start.file,
              start.line, start.func);
    return NULL;
  }
  // TODO: not great to pass this struct by value since I only need a few of the
  // fields here
  trace_task_region_attr_t task_attr;
  task_attr.type = otter_task_explicit;
  task_attr.id = otterTaskContext_get_task_context_id(task);
  task_attr.parent_id = otterTaskContext_get_parent_task_context_id(task);
  task_attr.flavour = otterTaskContext_get_task_flavour(task);
  task_attr.label_ref = otterTaskContext_get_task_label_ref(task);
  task_attr.init = otterTaskContext_get_init_location_ref(task);
  LOG_DEBUG("[%lu] begin task (child of %lu)", task_attr.id,
            task_attr.parent_id);
  otter_src_ref_t start_ref = get_source_location_ref((otter_src_location_t){
      .file = start.file, .func = start.func, .line = start.line});
  trace_graph_event_task_begin(get_thread_data()->location, task, task_attr,
                               start_ref);
  return task;
}

void otterTaskEnd(otter_task_context *task, otter_source_args end) {
  LOG_DEBUG("[%lu] end task", otterTaskContext_get_task_context_id(task));
  otter_src_ref_t end_ref = get_source_location_ref((otter_src_location_t){
      .file = end.file, .func = end.func, .line = end.line});
  trace_graph_event_task_end(get_thread_data()->location, task, end_ref);
  otterTaskContext_delete(task);
}

void otterTaskPushLabel(otter_task_context *task, const char *format, ...) {
  va_list args;
  va_start(args, format);
  otter_register_task_label_va_list(task, true, format, args);
  va_end(args);
  return;
}

otter_task_context *otterTaskPopLabel(const char *format, ...) {
  char label_buffer[LABEL_BUFFER_MAX_CHARS] = {0};
  va_list args;
  va_start(args, format);
  int chars_required =
      vsnprintf(&label_buffer[0], LABEL_BUFFER_MAX_CHARS, format, args);
  if (chars_required >= LABEL_BUFFER_MAX_CHARS) {
    LOG_WARN("label truncated (%d/%d chars written): %s",
             LABEL_BUFFER_MAX_CHARS, chars_required, label_buffer);
  }
  va_end(args);
  LOG_DEBUG("pop task with label: %s", label_buffer);
  TASK_MANAGER_LOCK();
  otter_task_context *task =
      trace_task_manager_pop_task(task_manager, label_buffer);
  TASK_MANAGER_UNLOCK();
  return task;
}

otter_task_context *otterTaskBorrowLabel(const char *format, ...) {
  char label_buffer[LABEL_BUFFER_MAX_CHARS] = {0};
  va_list args;
  va_start(args, format);
  int chars_required =
      vsnprintf(&label_buffer[0], LABEL_BUFFER_MAX_CHARS, format, args);
  if (chars_required >= LABEL_BUFFER_MAX_CHARS) {
    LOG_WARN("label truncated (%d/%d chars written): %s",
             LABEL_BUFFER_MAX_CHARS, chars_required, label_buffer);
  }
  va_end(args);
  LOG_DEBUG("pop task with label: %s", label_buffer);
  TASK_MANAGER_LOCK();
  otter_task_context *task =
      trace_task_manager_borrow_task(task_manager, label_buffer);
  TASK_MANAGER_UNLOCK();
  return task;
}

void otterSynchroniseTasks(otter_task_context *task, otter_task_sync_t mode) {
  LOG_DEBUG("synchronise tasks: %d", mode);

  if (task == NULL) {
    if (phase_task != NULL) {
      task = phase_task;
    } else {
      task = root_task;
    }
  }

  trace_sync_region_attr_t sync_attr;
  sync_attr.type = otter_sync_region_taskwait;
  sync_attr.sync_descendant_tasks =
      mode == otter_sync_descendants ? true : false;
  sync_attr.encountering_task_id = otterTaskContext_get_task_context_id(task);
  trace_graph_synchronise_tasks(get_thread_data()->location, task, sync_attr);
  return;
}

void otterTraceStart(void) { LOG_DEBUG("not currently implemented - ignored"); }

void otterTraceStop(void) { LOG_DEBUG("not currently implemented - ignored"); }

void otterPhaseBegin(const char *name, otter_source_args source) {
#if OTTER_USE_PHASES
  assert(name != NULL);
  assert(phase_task == NULL);
  assert(root_task != NULL);
  phase_task = otterTaskInitialise(root_task, 0, otter_no_add_to_pool, source,
                                   "OTTER PHASE: \"%s\" (%s:%d)", name,
                                   source.func, source.line);
  unique_id_t phase_id = otterTaskContext_get_task_context_id(phase_task);
  LOG_DEBUG("<phase %lu> OTTER PHASE: \"%s\" (%s:%d)", phase_id, name,
            source.func, source.line);
  otterTaskStart(phase_task, source);
#else
  LOG_WARN("phases are disabled - ignoring (name=%s)", name);
#endif
  return;
}

void otterPhaseEnd(otter_source_args source) {
#if OTTER_USE_PHASES
  assert(phase_task != NULL);
  unique_id_t phase_id = otterTaskContext_get_task_context_id(phase_task);
  LOG_DEBUG("<phase %lu> (%s:%d)", phase_id, source.func, source.line);
  otterTaskEnd(phase_task, source);
  phase_task = NULL;
#else
  LOG_WARN("phases are disabled - ignoring.");
#endif
  return;
}

void otterPhaseSwitch(const char *name, otter_source_args source) {
#if OTTER_USE_PHASES
  if (phase_task != NULL) {
    otterPhaseEnd(source);
  }
  otterPhaseBegin(name, source);
#else
  LOG_WARN("phases are disabled - ignoring (name=%s)", name);
#endif
  return;
}

static void debug_print_count(const char *str, int count, void *data) {
  LOG_DEBUG("%s %d", str, count);
  return;
}

static void debug_store_count_in_queue(const char *str, int count, void *data) {
  if (data == NULL) {
    return;
  }
  otter_queue_t *queue = (otter_queue_t *)data;
  queue_push(queue, (data_item_t){.ptr = str});
  queue_push(queue, (data_item_t){.value = count});
  return;
}
