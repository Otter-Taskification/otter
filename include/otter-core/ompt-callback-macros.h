#include <macros/debug.h>

#define OTTER_DEBUG(fmt, ...) LOG_DEBUG("[OTTer] " fmt PASS_ARGS(__VA_ARGS__));

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
