#include "public/otter-trace/trace-strings.h"

#include "trace-state.h"

otter_string_ref_t trace_get_string_ref(const char *string) {
  otter_string_ref_t string_ref = OTTER_STRING_UNDEFINED;
  pthread_mutex_lock(&state.strings.lock);
  string_ref = string_registry_insert(state.strings.instance, string);
  pthread_mutex_unlock(&state.strings.lock);
  return string_ref;
}
