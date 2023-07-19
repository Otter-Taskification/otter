/**
 * @file trace-state.h
 * @author Adam Tuft
 * @brief Defines the trace_state struct which gatyhers various objects which
 * represent the collective state of the application.
 * @version 0.1
 * @date 2023-04-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#if !defined(OTTER_TRACE_STATE_IMPL_H)
#define OTTER_TRACE_STATE_IMPL_H

#include "public/types/string_value_registry.hpp"
#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_GlobalDefWriter.h>
#include <pthread.h>

typedef struct trace_state_t {
  struct {
    OTF2_Archive *instance;
    // no lock
  } archive;
  struct {
    OTF2_GlobalDefWriter *instance;
    pthread_mutex_t lock;
  } global_def_writer;
  struct {
    string_registry *instance;
    pthread_mutex_t lock;
  } strings;
} trace_state_t;

#if defined(OTTER_TRACE_STATE_GLOBAL_DECL)
trace_state_t state = {
    {NULL},                            // archive
    {NULL, PTHREAD_MUTEX_INITIALIZER}, // global_def_writer
    {NULL, PTHREAD_MUTEX_INITIALIZER}  // strings
};
#else
extern trace_state_t state;
#endif

#endif // OTTER_TRACE_STATE_IMPL_H
