#include "public/otter-common.h"

/**
 * @brief Register a string with otter-trace and return its reference handle.
 * Identical strings will have the same reference.
 *
 * @param string The null-terminated string to register.
 * @return otter_string_ref_t The reference for this string.
 */
otter_string_ref_t trace_get_string_ref(const char *string);
