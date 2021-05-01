#if !defined(OTTER_COMMON_H)
#define OTTER_COMMON_H

/* 
    Definitions common to multiple modules
 */

#include <stdint.h>
#include <stdbool.h>

// Task tree use 9 bits to mask task type (extends ompt_task_flag_t)
#define TASK_TREE_TASK_TYPE_MASK 0x1FF 

/* Shift applied to mask task type when adding tasks to nodes in task tree */
#define TASK_TREE_TASK_TYPE_SHFT 60

/* Shift applied to mask parallel region id when adding tasks to nodes */
#define TASK_TREE_PARALLEL_ID_SHIFT 48

typedef uint64_t unique_id_t;

typedef struct otter_opt_t {
    char    *hostname;
    char    *graph_output;
    char    *graph_format;
    char    *graph_nodeattr;
    bool     append_hostname;
} otter_opt_t;

#endif // OTTER_COMMON_H
