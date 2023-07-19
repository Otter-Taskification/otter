#if !defined(OTTER_MACROS_CALLBACK_H)
#define OTTER_MACROS_CALLBACK_H

#include "public/debug.h"

/* Apply a macro to each of the possible values returned when setting a callback
   through ompt_set_callback */
#define FOREACH_OMPT_SET_RESULT(macro, result, event)                          \
  macro(ompt_set_error, 0, result, event)                                      \
      macro(ompt_set_never, 1, result, event)                                  \
          macro(ompt_set_impossible, 2, result, event)                         \
              macro(ompt_set_sometimes, 3, result, event)                      \
                  macro(ompt_set_sometimes_paired, 4, result, event)           \
                      macro(ompt_set_always, 5, result, event)

/* Print the result of attempting to set a callback. Useful as not all callbacks
   may be implemented */
#define print_matching_set_result(name, value, result, event)                  \
  do {                                                                         \
    if ((result == name))                                                      \
      fprintf(stderr, "%-32s | %-15s (%d)\n", #event, #name, result);          \
  } while (0);

/* Submit implemented callbacks to OMP and report the result */
#define set_callback(event, callback, id)                                      \
  do {                                                                         \
    if (callbacks.on_##event) {                                                \
      ompt_set_result_t r =                                                    \
          ompt_set_callback(event, (ompt_callback_t)callbacks.on_##event);     \
      FOREACH_OMPT_SET_RESULT(print_matching_set_result, r, event);            \
    }                                                                          \
  } while (0);

/* Report the return code from ompt_set_callback for a given event, then clear
   the callback */
#define report_callback(event, callback, id)                                   \
  do {                                                                         \
    ompt_set_result_t r =                                                      \
        ompt_set_callback(event, (ompt_callback_t)UINT64_MAX);                 \
    FOREACH_OMPT_SET_RESULT(print_matching_set_result, r, event);              \
    ompt_set_callback(event, (ompt_callback_t)NULL);                           \
  } while (0);

/* parallel region type in on_ompt_callback_parallel_begin */
#define LOG_DEBUG_PARALLEL_RGN_TYPE(flags, id)                                 \
  do {                                                                         \
    LOG_DEBUG_IF(flags &ompt_parallel_league, "%lu %-12s", id, "(teams)");     \
    LOG_DEBUG_IF(flags &ompt_parallel_team, "%lu %-12s", id, "(parallel)");    \
  } while (0);

/* print thread type in on_ompt_callback_thread_begin/end */
#define LOG_DEBUG_THREAD_TYPE(thread_type, id)                                 \
  do {                                                                         \
    LOG_DEBUG_IF(thread_type == ompt_thread_initial, "%lu %s ", id,            \
                 "(initial)");                                                 \
    LOG_DEBUG_IF(thread_type == ompt_thread_worker, "%lu %s ", id,             \
                 "(worker)");                                                  \
    LOG_DEBUG_IF(thread_type == ompt_thread_other, "%lu %s ", id, "(other)");  \
    LOG_DEBUG_IF(thread_type == ompt_thread_unknown, "%lu %s ", id,            \
                 "(unknown)");                                                 \
  } while (0);

/* print task type in on_ompt_callback_task_create */
#define LOG_DEBUG_TASK_TYPE(tid, parent, child, flags)                         \
  LOG_DEBUG("[t=%lu] %lu -> %lu %s", tid, parent, child,                       \
            flags &ompt_task_initial    ? "initial"                            \
            : flags &ompt_task_implicit ? "implicit"                           \
            : flags &ompt_task_explicit ? "explicit"                           \
            : flags &ompt_task_target   ? "target"                             \
                                        : "????")

#define LOG_DEBUG_IMPLICIT_TASK(flags, endpoint, id)                           \
  do {                                                                         \
    LOG_DEBUG_IF(flags &ompt_task_initial, "%s %lu %s", endpoint, id,          \
                 "initial");                                                   \
    LOG_DEBUG_IF(flags &ompt_task_implicit, "%s %lu %s", endpoint, id,         \
                 "implicit");                                                  \
    LOG_DEBUG_IF(flags &ompt_task_explicit, "%s %lu %s", endpoint, id,         \
                 "explicit");                                                  \
    LOG_DEBUG_IF(flags &ompt_task_target, "%s %lu %s", endpoint, id,           \
                 "target");                                                    \
  } while (0);

#define LOG_DEBUG_PRIOR_TASK_STATUS(status)                                    \
  do {                                                                         \
    LOG_DEBUG_IF(status == ompt_task_complete, "%s", "complete");              \
    LOG_DEBUG_IF(status == ompt_task_yield, "%s", "yield");                    \
    LOG_DEBUG_IF(status == ompt_task_cancel, "%s", "cancel");                  \
    LOG_DEBUG_IF(status == ompt_task_detach, "%s", "detach");                  \
    LOG_DEBUG_IF(status == ompt_task_early_fulfill, "%s", "early_fulfil");     \
    LOG_DEBUG_IF(status == ompt_task_late_fulfill, "%s", "late_fulfill");      \
    LOG_DEBUG_IF(status == ompt_task_switch, "%s", "switch");                  \
  } while (0);

#define LOG_DEBUG_WORK_TYPE(thread, wstype, count, endpoint)                   \
  do {                                                                         \
    LOG_DEBUG_IF(wstype == ompt_work_loop,                                     \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "loop", count);                                     \
    LOG_DEBUG_IF(wstype == ompt_work_sections,                                 \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "sections", count);                                 \
    LOG_DEBUG_IF(wstype == ompt_work_single_executor,                          \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "single", count);                                   \
    LOG_DEBUG_IF(wstype == ompt_work_single_other,                             \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "single (other)", count);                           \
    LOG_DEBUG_IF(wstype == ompt_work_workshare,                                \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "workshare", count);                                \
    LOG_DEBUG_IF(wstype == ompt_work_distribute,                               \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "distribute", count);                               \
    LOG_DEBUG_IF(wstype == ompt_work_taskloop,                                 \
                 "[t=%lu] (event) workshare-region-%s (%s %lu)", thread,       \
                 endpoint, "taskloop", count);                                 \
  } while (0);

#endif // OTTER_MACROS_CALLBACK_H
