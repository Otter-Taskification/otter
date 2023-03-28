#if !defined(OTTER_TRACE_RGN_MASTER_H)
#define OTTER_TRACE_RGN_MASTER_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include <omp-tools.h>
#include "public/otter-common.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace.h"

// TODO: once trace_region_def_t properly declared as opaque, combine all
// TODO: trace-region-*.h and trace-region-*.c into one header & src file which
// TODO: implements all variants of trace_region_def_t.

/* Create region */
trace_region_def_t *
trace_new_master_region(
    trace_location_def_t *loc,
    unique_id_t           encountering_task_id
);

/* Destroy region */
void trace_destroy_master_region(trace_region_def_t *rgn);

void trace_add_master_attributes(trace_region_def_t *rgn);

#endif // OTTER_TRACE_RGN_MASTER_H
