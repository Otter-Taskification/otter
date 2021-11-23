#if !defined(OTTER_TRACE_RGN_SYNC_H)
#define OTTER_TRACE_RGN_SYNC_H

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
trace_new_sync_region(
    trace_location_def_t *loc,
    ompt_sync_region_t    stype,                    // TODO: decouple
    unique_id_t           encountering_task_id
);

/* Destroy region */
void trace_destroy_sync_region(trace_region_def_t *rgn);

void trace_add_sync_attributes(trace_region_def_t *rgn);

#endif // OTTER_TRACE_RGN_SYNC_H
