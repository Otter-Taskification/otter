/**
 * @file trace-archive.c
 * @author Adam Tuft
 * @brief Responsible for initialising and finalising single instances of the
 * trace archive and its global definitions writer. Returns pointers to these
 * resources as well as mutexes protecting access to them both.
 */

#if !defined(OTTER_TRACE_ARCHIVE_IMPL_H)
#define OTTER_TRACE_ARCHIVE_IMPL_H

#include <otf2/OTF2_GeneralDefinitions.h>
#include <otf2/OTF2_GlobalDefWriter.h>
#include <pthread.h>

void trace_archive_write_string_ref(OTF2_GlobalDefWriter *def_writer,
                                    OTF2_StringRef ref, const char *s);

#endif // OTTER_TRACE_ARCHIVE_IMPL_H
