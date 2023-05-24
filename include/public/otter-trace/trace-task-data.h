#ifndef OTTER_TRACE_TASK_DATA_H
#define OTTER_TRACE_TASK_DATA_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-state.h"

typedef struct task_data_t task_data_t;

task_data_t *
new_task_data(
    trace_location_def_t  *loc,
    trace_region_def_t    *parent_task_region,
    otter_task_flag_t      flags,
    int                    has_dependences,
    otter_src_location_t  *src_location,
    const void            *task_create_ra
);

void
task_destroy(
    task_data_t *task_data
);

unique_id_t trace_task_get_id(task_data_t *task);
trace_region_def_t *trace_task_get_region_def(task_data_t *task);
otter_task_flag_t trace_task_get_flags(task_data_t *task);

#endif // OTTER_TRACE_TASK_DATA_H
