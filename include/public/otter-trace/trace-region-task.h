#if !defined(OTTER_TRACE_RGN_TASK_H)
#define OTTER_TRACE_RGN_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include <omp-tools.h>
#include "public/otter-common.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace.h"

/* Create region */
trace_region_def_t *
trace_new_task_region(
    trace_location_def_t *loc,
    trace_region_def_t   *parent_task_region,
    unique_id_t           task_id,
    otter_task_flag_t     flags,
    int                   has_dependences,
    otter_src_location_t *src_location,
    const void           *task_create_ra
);

/* Destroy region */
void trace_destroy_task_region(trace_region_def_t *rgn);

void trace_add_task_attributes(trace_region_def_t *rgn);

#endif // OTTER_TRACE_RGN_TASK_H
