/**
 * @file trace-src-ref.h
 * @author Adam Tuft
 * @brief Given file & function names, return a corresponding otter_src_ref_t.
 * @version 0.1
 * @date 2023-09-01
 *
 * @copyright Copyright (c) 2023 Adam Tuft
 *
 */

#include "public/otter-common.h"

otter_src_ref_t trace_src_loc_to_ref(const char *file, const char *func,
                                     int line);
