#include <macros/debug.h>

#define OTTER_DEBUG(fmt, ...) LOG_DEBUG("[OTTer] " fmt PASS_ARGS(__VA_ARGS__));

/* parallel region type in on_ompt_callback_parallel_begin */
#define LOG_DEBUG_PARALLEL_RGN_TYPE(flags, endpoint, id)                       \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_parallel_league,                                       \
        "%-8s %-12s %3lu %33s", endpoint, "teams", id, "");                    \
    LOG_DEBUG_IF(flags & ompt_parallel_team,                                         \
        "%-8s %-12s %3lu %33s", endpoint, "parallel", id, "");                 \
} while (0);

/* print thread type in on_ompt_callback_thread_begin/end */
#define LOG_DEBUG_THREAD_TYPE(thread_type, endpoint, id)                       \
do {                                                                           \
    LOG_DEBUG_IF(thread_type == ompt_thread_initial,                                  \
        "%-8s %-12s %lu %-9s %23s", endpoint, "thread", id, "(initial)", ""); \
    LOG_DEBUG_IF(thread_type == ompt_thread_worker,                                   \
        "%-8s %-12s %lu %-9s %23s", endpoint, "thread", id, "(worker)", "");  \
    LOG_DEBUG_IF(thread_type == ompt_thread_other,                                    \
        "%-8s %-12s %lu %-9s %23s", endpoint, "thread", id, "(other)", "");   \
    LOG_DEBUG_IF(thread_type == ompt_thread_unknown,                                  \
        "%-8s %-12s %lu %-9s %23s", endpoint, "thread", id, "(unknown)", ""); \
} while (0);

/* print task type in on_ompt_callback_task_create */
#define LOG_DEBUG_TASK_TYPE(flags)                                             \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_task_initial,  "%-8s %-12s %3lu %-12s %-20s",          \
        "", "create task", 0L, "(initial)", "");                               \
    LOG_DEBUG_IF(flags & ompt_task_implicit, "%-8s %-12s %3lu %-12s %-20s",          \
        "", "create task", 0L, "(implicit)", "");                              \
    LOG_DEBUG_IF(flags & ompt_task_explicit, "%-8s %-12s %3lu %-12s %-20s",          \
        "", "create task", 0L, "(explicit)", "");                              \
    LOG_DEBUG_IF(flags & ompt_task_target,   "%-8s %-12s %3lu %-12s %-20s",          \
        "", "create task", 0L, "(target)", "");                                \
} while (0); 

#define LOG_DEBUG_IMPLICIT_TASK(flags, endpoint)                               \
do {                                                                           \
    LOG_DEBUG_IF(flags & ompt_task_initial,   "%-8s %-50s",                          \
        endpoint, "initial task");                                             \
    LOG_DEBUG_IF(flags & ompt_task_implicit,  "%-8s %-50s",                          \
        endpoint, "implicit task");                                            \
    LOG_DEBUG_IF(flags & ompt_task_explicit,  "%-8s %-50s",                          \
        endpoint, "explicit task");                                            \
    LOG_DEBUG_IF(flags & ompt_task_target,    "%-8s %-50s",                          \
        endpoint, "target task");                                              \
} while (0); 

#define LOG_DEBUG_PRIOR_TASK_STATUS(status)                                    \
do {                                                                           \
                                                                               \
    LOG_DEBUG_IF(status == ompt_task_complete     , "%-8s %-50s",                    \
        "", "task complete");                                                  \
    LOG_DEBUG_IF(status == ompt_task_yield        , "%-8s %-50s",                    \
        "", "task yield");                                                     \
    LOG_DEBUG_IF(status == ompt_task_cancel       , "%-8s %-50s",                    \
        "", "task cancel");                                                    \
    LOG_DEBUG_IF(status == ompt_task_detach       , "%-8s %-50s",                    \
        "", "task detach");                                                    \
    LOG_DEBUG_IF(status == ompt_task_early_fulfill, "%-8s %-50s",                    \
        "", "task early_fulfil");                                              \
    LOG_DEBUG_IF(status == ompt_task_late_fulfill , "%-8s %-50s",                    \
        "", "task late_fulfill");                                              \
    LOG_DEBUG_IF(status == ompt_task_switch       , "%-8s %-50s",                    \
        "", "task switch");                                                    \
                                                                               \
} while (0);

#define LOG_DEBUG_WORK_TYPE(wstype, endpoint)                                  \
do {                                                                           \
    LOG_DEBUG_IF(wstype == ompt_work_loop           , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "loop");                                        \
    LOG_DEBUG_IF(wstype == ompt_work_sections       , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "sections");                                    \
    LOG_DEBUG_IF(wstype == ompt_work_single_executor, "%-8s %-25s %-24s",            \
        endpoint, "workshare", "single");                                      \
    LOG_DEBUG_IF(wstype == ompt_work_single_other   , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "single (other)");                              \
    LOG_DEBUG_IF(wstype == ompt_work_workshare      , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "workshare");                                   \
    LOG_DEBUG_IF(wstype == ompt_work_distribute     , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "distribute");                                  \
    LOG_DEBUG_IF(wstype == ompt_work_taskloop       , "%-8s %-25s %-24s",            \
        endpoint, "workshare", "taskloop");                                    \
} while(0);
