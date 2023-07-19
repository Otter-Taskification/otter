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

#include "public/otter-common.h"
#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_GlobalDefWriter.h>
#include <stdbool.h>

bool trace_initialise_archive(const char *archive_path,
                              const char *archive_name,
                              otter_event_model_t event_model,
                              OTF2_Archive **archive,
                              OTF2_GlobalDefWriter **global_def_writer);
bool trace_finalise_archive(OTF2_Archive *archive);

#endif // OTTER_TRACE_ARCHIVE_H
