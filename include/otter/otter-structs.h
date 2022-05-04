#if !defined(OTTER_STRUCTS_H)
#define OTTER_STRUCTS_H

#include <stdint.h>
#include <stdlib.h>
// #include "otter/otter-ompt-header.h"
// #include "otter/otter.h"
#include "otter/trace.h"

typedef struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;
    otter_thread_t        type;
    bool                  is_master_thread;   // of parallel region
} thread_data_t;

typedef struct task_data_t {
    unique_id_t         id;
    otter_task_flag_t   type;
    otter_task_flag_t   flags;
    trace_region_def_t *region;
} task_data_t;

typedef struct parallel_data_t {
    unique_id_t         id;
    unique_id_t         master_thread;
    task_data_t        *encountering_task_data;
    trace_region_def_t *region;
} parallel_data_t;

/* Parallel */
parallel_data_t *new_parallel_data(
    unique_id_t thread_id,
    unique_id_t encountering_task_id,
    task_data_t *encountering_task_data,
    unsigned int requested_parallelism,
    int flags);
void parallel_destroy(parallel_data_t *thread_data);

/* Thread */
thread_data_t *new_thread_data(otter_thread_t type);
void thread_destroy(thread_data_t *thread_data);

/* Task */
task_data_t *new_task_data(
    trace_location_def_t  *loc,
    trace_region_def_t    *parent_task_region,
    unique_id_t            task_id,
    otter_task_flag_t      flags,
    int                    has_dependences,
    otter_src_location_t  *src_location
);
void task_destroy(task_data_t *task_data);

/* Get new unique ID */
unique_id_t get_unique_parallel_id(void);
unique_id_t get_unique_thread_id(void);
unique_id_t get_unique_task_id(void);
unique_id_t get_dummy_time(void);

#endif // OTTER_STRUCTS_H
