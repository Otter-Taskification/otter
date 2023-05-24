#include "public/otter-trace/source-location.h"
#include "otter-trace/trace-state.h"

otter_src_ref_t
get_source_location_ref(otter_src_location_t location) {
    pthread_mutex_lock(&state.strings.lock);
    uint32_t file_ref = string_registry_insert(state.strings.instance, location.file);
    uint32_t func_ref = string_registry_insert(state.strings.instance, location.func);
    pthread_mutex_unlock(&state.strings.lock);
    return (otter_src_ref_t) {file_ref, func_ref, location.line};
}
