/**
 * @file trace-archive.h
 * @author Adam Tuft
 * @brief The public interface to initialise and finalise a tracing archive
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_ARCHIVE_H)
#define OTTER_TRACE_ARCHIVE_H

#include <stdbool.h>
#include "public/otter-common.h"

bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

#endif // OTTER_TRACE_ARCHIVE_H
