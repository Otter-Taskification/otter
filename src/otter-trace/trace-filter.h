#include <stdio.h>

#include "public/types/string_value_registry.hpp"

typedef struct trace_filter_t trace_filter_t;

/**
 * @brief Load a filter from `file`, converting strings into references using
 * string registry `s`.
 *
 * @param f A location to store the filter
 * @param s A string registry used to register filter strings
 * @param file The file to parse for the filter rules
 * @return int 0 on success
 */
int trace_filter_load(trace_filter_t **f, string_registry *s, FILE *file);
