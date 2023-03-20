#if !defined(OTTER_TRACE_STRUCTS_H)
#define OTTER_TRACE_STRUCTS_H

// TODO: this file justs couples together various components - consider removing altogether!

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <otf2/otf2.h>

#include "private/otter-ompt/otter-ompt-header.h"
#include "public/otter-common.h"
#include "public/types/queue.h"
#include "public/types/stack.h"
#include "public/otter-trace/trace.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-parallel.h"
#include "public/otter-trace/trace-region-workshare.h"
#include "public/otter-trace/trace-region-master.h"
#include "public/otter-trace/trace-region-sync.h"
#include "public/otter-trace/trace-region-task.h"
#include "public/otter-trace/trace-region-phase.h"

/* pretty-print region definitions */
// void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_STRUCTS_H
