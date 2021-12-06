#if !defined(OTTER_STRUCTS_H)
#define OTTER_STRUCTS_H

#include <stdint.h>
#include <stdlib.h>
// #include "otter/otter-ompt-header.h"
#include "otter/otter.h"
#include "otter/trace.h"

/* Used as an array index to keep track of unique ids for different entities */
typedef enum unique_id_type_t {
    id_timestamp        ,
    id_parallel         ,
    id_thread           ,
    id_task             ,
    NUM_ID_TYPES
} unique_id_type_t;
#define get_unique_parallel_id() get_unique_id(id_parallel)
#define get_unique_thread_id()   get_unique_id(id_thread)
#define get_unique_task_id()     get_unique_id(id_task)
#define get_dummy_time()         get_unique_id(id_timestamp)

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
thread_data_t *new_thread_data(otter_thread_t type);
void thread_destroy(thread_data_t *thread_data);
struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;
    otter_thread_t        type;
    bool                  is_master_thread;   // of parallel region
};

/* Task */
task_data_t *new_task_data(
    trace_location_def_t  *loc,
    trace_region_def_t    *parent_task_region,
    unique_id_t            task_id,
    otter_task_flag_t      flags,
    int                    has_dependences
);
void task_destroy(task_data_t *task_data);
struct task_data_t {
    unique_id_t         id;
    otter_task_flag_t   type;
    otter_task_flag_t   flags;
    trace_region_def_t *region;
};

unique_id_t get_unique_id(unique_id_type_t id_type);

#endif // OTTER_STRUCTS_H
