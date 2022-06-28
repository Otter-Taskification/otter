#if !defined(OTTER_TRACE_RGN_PHASE_H)
#define OTTER_TRACE_RGN_PHASE_H

#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/trace.h"

/* Create region */
trace_region_def_t *
trace_new_phase_region(
    trace_location_def_t *loc,
    otter_phase_region_t  type,
    unique_id_t           encountering_task_id,
    const char           *phase_name
);

/* Destroy region */
void trace_destroy_phase_region(trace_region_def_t *rgn);

void trace_add_phase_attributes(trace_region_def_t *rgn);

#endif // OTTER_TRACE_RGN_PHASE_H
