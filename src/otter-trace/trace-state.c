#include <stdlib.h>
#include <pthread.h>
#include "otter-trace/trace-state.h"

struct trace_state_t {
    OTF2_Archive *archive;
    OTF2_GlobalDefWriter *global_def_writer;
    string_registry *registry;
    pthread_mutex_t lock_global_def_writer;
    pthread_mutex_t lock_registry;
};

trace_state_t *
trace_state_new(OTF2_Archive *archive, OTF2_GlobalDefWriter *global_def_writer, string_registry *registry)
{
    trace_state_t *state = malloc(sizeof(trace_state_t));
    *state = (trace_state_t) {
        .archive = archive,
        .global_def_writer = global_def_writer,
        .registry = registry,
        .lock_global_def_writer = PTHREAD_MUTEX_INITIALIZER,
        .lock_registry = PTHREAD_MUTEX_INITIALIZER
    };
    return state;
}

void 
trace_state_destroy(trace_state_t *state)
{
    free((void*)state);
}

OTF2_Archive *
trace_state_get_archive(trace_state_t * state)
{
    return state->archive;
}

OTF2_GlobalDefWriter *
trace_state_get_global_def_writer(trace_state_t * state)
{
    return state->global_def_writer;
}

void 
trace_state_lock_global_def_writer(trace_state_t * state)
{
    pthread_mutex_lock(&(state->lock_global_def_writer));
}

void 
trace_state_unlock_global_def_writer(trace_state_t * state)
{
    pthread_mutex_unlock(&(state->lock_global_def_writer));
}

string_registry *
trace_state_get_string_registry(trace_state_t * state)
{
    return state->registry;
}

void 
trace_state_lock_string_registry(trace_state_t * state)
{
    pthread_mutex_lock(&(state->lock_registry));
}

void 
trace_state_unlock_string_registry(trace_state_t * state)
{
    pthread_mutex_unlock(&(state->lock_registry));
}
