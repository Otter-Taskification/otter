#include "ompt-common.h"

/* Parallel region type */
typedef struct parallel_data_t {
    unique_id_t         id;
} parallel_data_t;

/* Thread type */
typedef struct thread_data_t {
    unique_id_t         id;
} thread_data_t;

/* Task type */
typedef struct task_data_t {
    unique_id_t         id;
} task_data_t;
