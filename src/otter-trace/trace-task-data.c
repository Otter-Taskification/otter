#include <stdlib.h>
#include "public/otter-trace/trace-task-data.h"
#include "otter-trace/trace-get-unique-id.h"
typedef struct task_data_t {
    unique_id_t         id;
    otter_task_flag_t   type;
    otter_task_flag_t   flags;
    trace_region_def_t *region;
} task_data_t;

static unique_id_t get_unique_task_id(void) {
    return get_unique_id();
}

task_data_t *
new_task_data(
    trace_location_def_t *loc,
    trace_region_def_t   *parent_task_region,
    otter_task_flag_t     flags,
    int                   has_dependences,
    otter_src_location_t *src_location,
    const void           *task_create_ra)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id     = get_unique_task_id(),
        .type   = flags & otter_task_type_mask,
        .flags  = flags,
        .region = NULL
    };
    new->region = trace_new_task_region(
        loc, 
        parent_task_region, 
        new->id,
        flags, 
        has_dependences,
        src_location,
        task_create_ra
    );
    return new;
}

void task_destroy(task_data_t *task_data)
{
    free(task_data);
    return;
}

unique_id_t
trace_task_get_id(task_data_t *task) {
    return task->id;
}

trace_region_def_t *
trace_task_get_region_def(task_data_t *task) {
    return task->region;
}

otter_task_flag_t
trace_task_get_flags(task_data_t *task) {
    return task->flags;
}
