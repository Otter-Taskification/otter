#if !defined(OTTER_STRUCTS_H)
#define OTTER_STRUCTS_H

#include <stdint.h>
#include <stdlib.h>
#include <otter-ompt-header.h>
#include <otter-core/otter.h>
#include <otter-trace/trace.h>

/* forward declarations */
typedef struct parallel_data_t parallel_data_t;
typedef struct thread_data_t thread_data_t;
typedef struct task_data_t task_data_t;
typedef struct scope_t scope_t;

/* Parallel */
parallel_data_t *new_parallel_data(
    unique_id_t thread_id,
    unique_id_t encountering_task_id,
    task_data_t *encountering_task_data,
    unsigned int requested_parallelism,
    int flags);
void parallel_destroy(parallel_data_t *thread_data);
struct parallel_data_t {
    unique_id_t         id;
    unique_id_t         master_thread;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
};

/* Thread */
thread_data_t *new_thread_data(ompt_thread_t type);
void thread_destroy(thread_data_t *thread_data);
struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;
    ompt_thread_t         type;
    bool                  is_master_thread;   // of parallel region
};

/* Task */
task_data_t *new_task_data(trace_location_def_t *loc,trace_region_def_t *parent_task_region, unique_id_t task_id, ompt_task_flag_t flags, int has_dependences);
void task_destroy(task_data_t *task_data);
struct task_data_t {
    unique_id_t         id;
    ompt_task_flag_t    type;
    ompt_task_flag_t    flags;
    trace_region_def_t *region;
};

#endif // OTTER_STRUCTS_H
