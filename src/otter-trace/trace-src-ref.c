#include "public/otter-trace/trace-src-ref.h"

#include "trace-src-ref.h"
#include "trace-state.h"

otter_src_ref_t trace_src_loc_to_ref(const char *file, const char *func,
                                     int line) {
  pthread_mutex_lock(&state.strings.lock);
  otter_src_ref_t ref =
      get_source_location_ref(state.strings.instance, file, func, line);
  pthread_mutex_unlock(&state.strings.lock);
  return ref;
}

otter_src_ref_t get_source_location_ref(string_registry *strings,
                                        const char *file, const char *func,
                                        int line) {
  otter_src_ref_t ref;
  ref.file = string_registry_insert(strings, file);
  ref.func = string_registry_insert(strings, func);
  ref.line = line;
  return ref;
}
