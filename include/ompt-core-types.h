#if !defined(OMPT_CORE_TYPES_H)
#define OMPT_CORE_TYPES_H

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

#include <ompt-common.h>
#include <pthread.h>

#include <modules/task-tree.h>

#define TASK_TYPE_BITS      0xF

/* forward declarations */
typedef struct parallel_data_t parallel_data_t;
typedef struct thread_data_t thread_data_t;
typedef struct task_data_t task_data_t;

/* Parallel region type */
typedef struct parallel_data_t {
    unique_id_t         id;
    task_data_t        *encountering_task_data;
} parallel_data_t;

/* Thread type */
typedef struct thread_data_t {
    unique_id_t         id;
} thread_data_t;

/* Task type */
typedef struct task_data_t {
    unique_id_t         id;
    ompt_task_flag_t    type;
    tree_node_t        *tree_node;

    /* only accessed by implicit tasks which are children of an initial task to
       atomically register as children of the initial task. This is because the
       implicit-task-begin event happens in the context of the implicit child
       task rather than in that of the initial parent task
     */
    pthread_mutex_t    *lock;
} task_data_t;

#endif // OMPT_CORE_TYPES_H
