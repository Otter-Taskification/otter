/**
 * @file trace-archive.h
 * @author Adam Tuft
 * @brief Private interface to initialise and finalise an archive.
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

bool trace_initialise_archive(const char *archive_path, const char *archive_name, otter_event_model_t event_model);
bool trace_finalise_archive(void);

#endif // OTTER_TRACE_ARCHIVE_H
