#if !defined(OTTER_H)
#define OTTER_H

#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// getrusage
#include <sys/resource.h>
#include <sys/time.h>

#include <omp-tools.h>

#include "otter-entry.h"
#include "public/otter-common.h"

/* A naming convention was chosen for the callbacks so that each callback is
   named after the event in ompt.h which it handles. The struct passed to the
   tool by ompt-core has a member for each OMP event. This macro attaches
   a callback to its corresponding field in that struct.
*/
#define include_callback(callbacks, event) callbacks->on_##event = on_##event

#define STR_EQUAL(a, b) (!strcmp(a, b))

/* Include the function prototypes for the callbacks this tool implements */
#define implements_callback_thread_begin
#define implements_callback_thread_end
#define implements_callback_parallel_begin
#define implements_callback_parallel_end
#define implements_callback_task_create
#define implements_callback_task_schedule
#define implements_callback_implicit_task
#define implements_callback_work
#define implements_callback_sync_region
#define implements_callback_master
#include "ompt-callback-prototypes.h"

#endif // OTTER_H
