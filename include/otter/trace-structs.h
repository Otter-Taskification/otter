#if !defined(OTTER_TRACE_STRUCTS_H)
#define OTTER_TRACE_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <otf2/otf2.h>

#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"
#include "otter/queue.h"
#include "otter/stack.h"
#include "otter/trace.h"
#include "otter/trace-location.h"
#include "otter/trace-region-parallel.h"
#include "otter/trace-region-workshare.h"
#include "otter/trace-region-master.h"
#include "otter/trace-region-sync.h"
#include "otter/trace-region-task.h"

/* pretty-print region definitions */
// void trace_region_pprint(FILE *fp, trace_region_def_t *r, const char func[], const int line);

#endif // OTTER_TRACE_STRUCTS_H
