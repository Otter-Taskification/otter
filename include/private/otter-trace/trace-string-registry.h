#if !defined(OTTER_TRACE_STRING_REGISTRY_H)
#define OTTER_TRACE_STRING_REGISTRY_H

#include "public/otter-common.h"
#include "public/types/value_registry.hpp"

void trace_init_str_registry(string_registry *registry);
void trace_destroy_str_registry(void);
string_registry *trace_get_str_registry(void);
uint32_t trace_register_string(const char *str);
uint32_t trace_register_string_with_lock(const char *str);

#endif // OTTER_TRACE_STRING_REGISTRY_H
