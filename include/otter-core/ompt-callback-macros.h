#if !defined(OMPT_CALLBACK_MACROS_H)
#define OMPT_CALLBACK_MACROS_H

#include <macros/debug.h>

/* ancestor level for innermost parallel region */
#define INNER 0

/* return values from ompt_get_parallel_info_t */
#define PARALLEL_INFO_AVAIL     2
#define PARALLEL_INFO_UNAVAIL   1
#define PARALLEL_INFO_NONE      0

/* Packing task type & parallel region into task ID value

   Have 64 bits available in unique_id_t/tree_node_id_t/array_id_t but never
   going to need all of them for a unique task ID in any realistic scenario

   => use some of the bits to also pass to task-tree a task's type & parallel
        region

   task type:       0xf000000000000000 => 16 values (60-bit shift)
   parallel region: 0x00ff000000000000 => 255 values (48-bit shift)

    __builtin_ctzll - Returns the number of trailing 0-bits in x, starting at 
    the least significant bit position. If x is 0, the result is undefined

    I use this built-in to convert ompt_task_flag_t to an int representing the
    bit that is set i.e. 0x01 -> 0, 0x08 -> 3 etc. This converts a value like
    0b1000 into 0b0011 which requires fewer bits. This means I can represent up
    to 16 task types in the top 4 bits of the task ID, instead of setting 16
    independent bits

    NOTE: need to check whether this is portable between clang & icc

 */
#define PACK_TASK_BITS(flags, task_id, parallel_id)                           \
    (task_id \
        | ( (unique_id_t)__builtin_ctzll(flags) << TASK_TREE_TASK_TYPE_SHFT ) \
        | ((parallel_id & 0xFF)<<TASK_TREE_PARALLEL_ID_SHIFT) )

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

#define LOG_DEBUG_WORK_TYPE(thread, wstype, count, endpoint)                   \
do {                                                                           \
    LOG_DEBUG_IF(wstype == ompt_work_loop           , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "loop", count);                         \
    LOG_DEBUG_IF(wstype == ompt_work_sections       , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "sections", count);                     \
    LOG_DEBUG_IF(wstype == ompt_work_single_executor, "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "single", count);                       \
    LOG_DEBUG_IF(wstype == ompt_work_single_other   , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "single (other)", count);               \
    LOG_DEBUG_IF(wstype == ompt_work_workshare      , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "workshare", count);                    \
    LOG_DEBUG_IF(wstype == ompt_work_distribute     , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "distribute", count);                   \
    LOG_DEBUG_IF(wstype == ompt_work_taskloop       , "[%lu] %s %s %s %lu",    \
        thread, endpoint, "workshare", "taskloop", count);                     \
} while(0);

#endif // OMPT_CALLBACK_MACROS_H
