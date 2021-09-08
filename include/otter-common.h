#if !defined(OTTER_COMMON_H)
#define OTTER_COMMON_H

/* 
    Definitions common to multiple modules
 */

#include <stdint.h>
#include <stdbool.h>

#include <assert.h>

// Bits used by ompt_task_flag_t to indicate task type
#define OMPT_TASK_TYPE_BITS 0x0F

typedef uint64_t unique_id_t;

typedef struct otter_opt_t {
    char    *hostname;
    char    *tracename;
    char    *tracepath;
    char    *archive_name;
    bool     append_hostname;
} otter_opt_t;

// ompt_task_flag_t
#define OMPT_TASK_TYPE_TO_STR(type)                                            \
    ((type) == ompt_task_initial  ? "ompt_task_initial":                       \
     (type) == ompt_task_implicit ? "ompt_task_implicit":                      \
     (type) == ompt_task_explicit ? "ompt_task_explicit":                      \
     (type) == ompt_task_target   ? "ompt_task_target" : "unknown" )

// ompt_sync_region_t
#define OMPT_SYNC_TYPE_TO_STR(type)                                                                  \
    ((type) == ompt_sync_region_barrier                ? "ompt_sync_region_barrier" :                \
     (type) == ompt_sync_region_barrier_implicit       ? "ompt_sync_region_barrier_implicit" :       \
     (type) == ompt_sync_region_barrier_explicit       ? "ompt_sync_region_barrier_explicit" :       \
     (type) == ompt_sync_region_barrier_implementation ? "ompt_sync_region_barrier_implementation" : \
     (type) == ompt_sync_region_taskwait               ? "ompt_sync_region_taskwait" :               \
     (type) == ompt_sync_region_taskgroup              ? "ompt_sync_region_taskgroup" :              \
     (type) == ompt_sync_region_reduction              ? "ompt_sync_region_reduction" : "unknown")

// ompt_work_t
#define OMPT_WORK_TYPE_TO_STR(type)                                            \
    ((type) == ompt_work_loop            ? "ompt_work_loop" :                  \
     (type) == ompt_work_sections        ? "ompt_work_sections" :              \
     (type) == ompt_work_single_executor ? "ompt_work_single_executor" :       \
     (type) == ompt_work_single_other    ? "ompt_work_single_other" :          \
     (type) == ompt_work_workshare       ? "ompt_work_workshare" :             \
     (type) == ompt_work_distribute      ? "ompt_work_distribute" :            \
     (type) == ompt_work_taskloop        ? "ompt_work_taskloop" : "unknown")

#endif // OTTER_COMMON_H
