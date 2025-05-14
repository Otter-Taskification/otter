#include "public/feature-macros.h"
#include <unistd.h>
#include <sys/syscall.h>

#if !defined(SYS_gettid)
#error No SYS_gettid
#endif

#include <inttypes.h>
#include <stdlib.h>

#define OTTER_USE_PRIVATE_HEADER
#include "public/otter-trace/trace-thread-data.h"
#include "trace-get-unique-id.h"
#include "trace-static-constants.h"

thread_data_t *new_thread_data(otter_thread_t type) {
    thread_data_t *thread_data = malloc(sizeof(*thread_data));
    *thread_data = (thread_data_t){
        .id = get_unique_id(),
        .location = NULL,
        .type = type,
        .is_master_thread = false,
        .active_task = NULL, // threads start out without holding any specific task
        .tid = syscall(SYS_gettid),
    };

    /* Create a location definition for this thread */
    thread_data->location =
        trace_new_location_definition(thread_data->id, type, OTF2_LOCATION_TYPE_CPU_THREAD, DEFAULT_LOCATION_GRP);

    LOG_INFO("create thread %" PRIu64 ", gettid=%d", thread_data->id, thread_data->tid);

    return thread_data;
}

void thread_destroy(thread_data_t *thread_data) {
    trace_destroy_location(thread_data->location);
    free(thread_data);
}
