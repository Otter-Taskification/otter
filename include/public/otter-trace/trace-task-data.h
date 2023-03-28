#ifndef OTTER_TRACE_TASK_DATA_H
#define OTTER_TRACE_TASK_DATA_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"

// TODO: make this struct fully opaque by adding getters and moving definition into trace-task-data.c
typedef struct task_data_t {
    unique_id_t         id;
    otter_task_flag_t   type;
    otter_task_flag_t   flags;
    trace_region_def_t *region;
} task_data_t;

task_data_t *
new_task_data(
    trace_location_def_t  *loc,
    trace_region_def_t    *parent_task_region,
    unique_id_t            task_id,
    otter_task_flag_t      flags,
    int                    has_dependences,
    otter_src_location_t  *src_location,
    const void            *task_create_ra
);

void
task_destroy(
    task_data_t *task_data
);

// TODO: factor out of this interface, doesn't need to be public (only used when `new_task_data` called)
unique_id_t
get_unique_task_id(
    void
);

#endif // OTTER_TRACE_TASK_DATA_H
