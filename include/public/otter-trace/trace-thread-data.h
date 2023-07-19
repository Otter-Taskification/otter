#ifndef OTTER_TRACE_THREAD_DATA_H
#define OTTER_TRACE_THREAD_DATA_H

#include "public/otter-common.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-state.h"
#include "public/otter-trace/trace-types.h"

// TODO: can this struct be made opaque?
typedef struct thread_data_t {
  unique_id_t id;
  trace_location_def_t *location;
  otter_thread_t type;
  bool is_master_thread; // of parallel region
} thread_data_t;

thread_data_t *new_thread_data(otter_thread_t type);

void thread_destroy(thread_data_t *thread_data);

#endif // OTTER_TRACE_THREAD_DATA_H
