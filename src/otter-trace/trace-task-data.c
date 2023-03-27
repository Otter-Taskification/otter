#include <stdlib.h>
#include "public/otter-trace/trace-task-data.h"
#include "public/otter-trace/trace-region-task.h"
#include "src/otter-trace/trace-get-unique-id.h"

// TODO: otter-trace shouldn't be depending on OMPT concerns - refactor task_data_t so it doesn't depend on OMPT
// Bits used by ompt_task_flag_t to indicate task type
#define OMPT_TASK_TYPE_BITS 0x0F

unique_id_t get_unique_task_id(void) {
    return get_unique_id();
}

task_data_t *
new_task_data(
    trace_location_def_t *loc,
    trace_region_def_t   *parent_task_region,
    unique_id_t           task_id,
    otter_task_flag_t     flags,
    int                   has_dependences,
    otter_src_location_t *src_location,
    const void           *task_create_ra)
{
    task_data_t *new = malloc(sizeof(*new));
    *new = (task_data_t) {
        .id     = task_id,
        .type   = flags & OMPT_TASK_TYPE_BITS,
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
