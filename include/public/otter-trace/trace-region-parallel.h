#if !defined(OTTER_TRACE_RGN_PARALLEL_H)
#define OTTER_TRACE_RGN_PARALLEL_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include "private/otter-ompt/otter-ompt-header.h"
#include "public/otter-common.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace.h"

/* Create new region */
trace_region_def_t *
trace_new_parallel_region(
    unique_id_t    id, 
    unique_id_t    master,
    unique_id_t    encountering_task_id,
    int            flags,
    unsigned int   requested_parallelism
);

/* Destroy parallel region */
void trace_destroy_parallel_region(trace_region_def_t *rgn);

void trace_add_parallel_attributes(trace_region_def_t *rgn);

#endif // OTTER_TRACE_RGN_PARALLEL_H
