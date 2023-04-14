#include <stdlib.h>
#include "public/otter-trace/trace-thread-data.h"
#include "otter-trace/trace-get-unique-id.h"
#include "otter-trace/trace-static-constants.h"

// TODO: accept injected state
thread_data_t *
new_thread_data(otter_thread_t type)
{
    thread_data_t *thread_data = malloc(sizeof(*thread_data));
    *thread_data = (thread_data_t) {
        .id                 = get_unique_id(),
        .location           = NULL,
        .type               = type,
        .is_master_thread   = false
    };

    /* Create a location definition for this thread */
    // TODO: pass state here
    thread_data->location = trace_new_location_definition(
        thread_data->id,
        type,
        OTF2_LOCATION_TYPE_CPU_THREAD,
        DEFAULT_LOCATION_GRP);

    return thread_data;
}

void 
thread_destroy(thread_data_t *thread_data)
{
    trace_destroy_location(thread_data->location);
    free(thread_data);
}
