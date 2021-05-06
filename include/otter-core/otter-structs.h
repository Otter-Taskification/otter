#if !defined(OTTER_STRUCTS_H)
#define OTTER_STRUCTS_H

#include <stdint.h>
#include <stdlib.h>
#include <otter-ompt-header.h>
#include <otter-core/otter.h>

/* forward declarations */
typedef struct parallel_data_t parallel_data_t;
typedef struct thread_data_t thread_data_t;
typedef struct task_data_t task_data_t;
typedef struct scope_t scope_t;

/* Parallel */
parallel_data_t *new_parallel_data(int flags);
void parallel_destroy(parallel_data_t *thread_data);
struct parallel_data_t {
    unique_id_t         id;
    int                 flags;
    unsigned int        actual_parallelism;
    trace_region_def_t *region;
    scope_t            *scope;
};

/* Thread */
thread_data_t *new_thread_data(ompt_thread_t type);
void thread_destroy(thread_data_t *thread_data);
struct thread_data_t {
    unique_id_t           id;
    trace_location_def_t *location;
    ompt_thread_t         type;
    stack_t              *region_scope_stack; // sequence of nested scopes
    scope_t              *prior_scope;        // most recently pushed/popped from stack
    bool                  is_master_thread;   // of parallel region
    bool                  is_single;          // in single region
    unsigned int          actual_parallelism; // in current parallel region
    unsigned int          index;              // in current parallel region
    queue_t              *sync_node_queue;    // master thread collects sync nodes
};

/* Task */
task_data_t *new_task_data(unique_id_t id, ompt_task_flag_t flags);
void task_destroy(task_data_t *task_data);
struct task_data_t {
    unique_id_t         id;
    task_graph_node_t  *task_node_ref;
    ompt_task_flag_t    type;
    ompt_task_flag_t    flags;
    scope_t            *scope; // inherited from parent task

};

/* Scope */
scope_t *new_scope(scope_type_t type, void *data);
void scope_destroy(scope_t *scope);
struct scope_t {
    scope_type_t            type;
    ompt_scope_endpoint_t   endpoint;
    void                   *data;
    stack_t                *task_graph_nodes;
    task_graph_node_t      *begin_node;
    task_graph_node_t      *end_node;
    pthread_mutex_t         lock;
};

#endif // OTTER_STRUCTS_H
