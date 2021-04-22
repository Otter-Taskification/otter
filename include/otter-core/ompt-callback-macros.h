#if !defined(OMPT_CALLBACK_MACROS_H)
#define OMPT_CALLBACK_MACROS_H

#include <macros/debug.h>

/* ancestor level for innermost parallel region */
#define INNER 0

/* return values from ompt_get_parallel_info_t */
#define PAR_INFO_AVAIL          2
#define PAR_INFO_UNAVAIL        1
#define PAR_INFO_NONE           0

/* pack task type and enclosing paralle region into child ID bits to pass to
   tree_add_child_to_node

   task type (top 4 bits):    0xf000000000000000 (7.5-byte shift)
   enclosing parallel region: 0x00ff000000000000 (6-byte shift MAX 256 REGIONS!) 
 */
#define PACK_CHILD_TASK_BITS(flags, child_id, parallel_id)                     \
    (child_id | (((unique_id_t)flags & 0x0F)<<60) | ((parallel_id & 0xFF)<<48))

/* Apply a macro to each of the possible values returned when setting a callback
   through ompt_set_callback */
#define FOREACH_OMPT_SET_RESULT(macro, result, event)                          \
    macro(ompt_set_error,             0, result, event)                        \
    macro(ompt_set_never,             1, result, event)                        \
    macro(ompt_set_impossible,        2, result, event)                        \
    macro(ompt_set_sometimes,         3, result, event)                        \
    macro(ompt_set_sometimes_paired,  4, result, event)                        \
    macro(ompt_set_always,            5, result, event)

/* Print the result of attempting to set a callback. Useful as not all callbacks
   may be implemented */
#define print_matching_set_result(name, value, result, event)                  \
    do {                                                                       \
        if ((result == name))                                                  \
            fprintf(stderr, "%-32s -> %s (%d)\n", #event, #name, result);      \
    } while(0);

/* Submit implemented callbacks to OMP and report the result */
#define set_callback(event, callback, id)                                      \
    do {                                                                       \
        if(callbacks.on_##event) {                                             \
            ompt_set_result_t r = ompt_set_callback(                           \
                event, (ompt_callback_t) callbacks.on_##event);                \
            FOREACH_OMPT_SET_RESULT(print_matching_set_result, r, event);      \
        }                                                                      \
    } while(0);

/* parallel region type in on_ompt_callback_parallel_begin */
#define LOG_DEBUG_PARALLEL_RGN_TYPE(flags, id)                                 \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_parallel_league,                                 \
        "%lu %-12s", id, "(teams)");                                           \
    LOG_DEBUG_IF(flags & ompt_parallel_team,                                   \
        "%lu %-12s", id, "(parallel)");                                        \
} while (0);

/* print thread type in on_ompt_callback_thread_begin/end */
#define LOG_DEBUG_THREAD_TYPE(thread_type, id)                                 \
do {                                                                           \
    LOG_DEBUG_IF(thread_type == ompt_thread_initial,                           \
        "%lu %s ", id, "(initial)");                                           \
    LOG_DEBUG_IF(thread_type == ompt_thread_worker,                            \
        "%lu %s ", id, "(worker)");                                            \
    LOG_DEBUG_IF(thread_type == ompt_thread_other,                             \
        "%lu %s ", id, "(other)");                                             \
    LOG_DEBUG_IF(thread_type == ompt_thread_unknown,                           \
        "%lu %s ", id, "(unknown)");                                           \
} while (0);

/* print task type in on_ompt_callback_task_create */
#define LOG_DEBUG_TASK_TYPE(parent, child, flags)                              \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_task_initial,                                    \
        "%lu -> %lu %s", parent, child, "initial");                            \
    LOG_DEBUG_IF(flags & ompt_task_implicit,                                   \
        "%lu -> %lu %s", parent, child, "implicit");                           \
    LOG_DEBUG_IF(flags & ompt_task_explicit,                                   \
        "%lu -> %lu %s", parent, child, "explicit");                           \
    LOG_DEBUG_IF(flags & ompt_task_target,                                     \
        "%lu -> %lu %s", parent, child, "target");                             \
} while (0); 

#define LOG_DEBUG_IMPLICIT_TASK(flags, endpoint, id)                           \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_task_initial,                                    \
        "%s %lu %s", endpoint, id, "initial");                                 \
    LOG_DEBUG_IF(flags & ompt_task_implicit,                                   \
        "%s %lu %s", endpoint, id, "implicit");                                \
    LOG_DEBUG_IF(flags & ompt_task_explicit,                                   \
        "%s %lu %s", endpoint, id, "explicit");                                \
    LOG_DEBUG_IF(flags & ompt_task_target,                                     \
        "%s %lu %s", endpoint, id, "target");                                  \
} while (0); 

#define LOG_DEBUG_PRIOR_TASK_STATUS(status)                                    \
do {                                                                           \
    LOG_DEBUG_IF(status == ompt_task_complete     , "%s", "complete");         \
    LOG_DEBUG_IF(status == ompt_task_yield        , "%s", "yield");            \
    LOG_DEBUG_IF(status == ompt_task_cancel       , "%s", "cancel");           \
    LOG_DEBUG_IF(status == ompt_task_detach       , "%s", "detach");           \
    LOG_DEBUG_IF(status == ompt_task_early_fulfill, "%s", "early_fulfil");     \
    LOG_DEBUG_IF(status == ompt_task_late_fulfill , "%s", "late_fulfill");     \
    LOG_DEBUG_IF(status == ompt_task_switch       , "%s", "switch");           \
} while (0);

#define LOG_DEBUG_WORK_TYPE(wstype, endpoint)                                  \
do {                                                                           \
    LOG_DEBUG_IF(wstype == ompt_work_loop           , "%s %s %s",              \
        endpoint, "workshare", "loop");                                        \
    LOG_DEBUG_IF(wstype == ompt_work_sections       , "%s %s %s",              \
        endpoint, "workshare", "sections");                                    \
    LOG_DEBUG_IF(wstype == ompt_work_single_executor, "%s %s %s",              \
        endpoint, "workshare", "single");                                      \
    LOG_DEBUG_IF(wstype == ompt_work_single_other   , "%s %s %s",              \
        endpoint, "workshare", "single (other)");                              \
    LOG_DEBUG_IF(wstype == ompt_work_workshare      , "%s %s %s",              \
        endpoint, "workshare", "workshare");                                   \
    LOG_DEBUG_IF(wstype == ompt_work_distribute     , "%s %s %s",              \
        endpoint, "workshare", "distribute");                                  \
    LOG_DEBUG_IF(wstype == ompt_work_taskloop       , "%s %s %s",              \
        endpoint, "workshare", "taskloop");                                    \
} while(0);

#endif // OMPT_CALLBACK_MACROS_H
