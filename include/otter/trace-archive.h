/**
 * @file trace-archive.c
 * @author Adam Tuft
 * @brief Responsible for initialising and finalising single instances of the 
 * trace archive and its global definitions writer. Returns pointers to these 
 * resources as well as mutexes protecting access to them both.
 */

#if !defined(OTTER_TRACE_ARCHIVE_H)
#define OTTER_TRACE_ARCHIVE_H

#include <stdbool.h>
#include <otf2/otf2.h>
#include "otter/otter-common.h"

pthread_mutex_t *global_def_writer_lock(void);
pthread_mutex_t *global_archive_lock(void);

OTF2_GlobalDefWriter *get_global_def_writer(void);
OTF2_Archive *get_global_archive(void);

/* interface function prototypes */
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

#endif // OTTER_TRACE_ARCHIVE_H
