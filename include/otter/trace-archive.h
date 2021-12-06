#if !defined(OTTER_TRACE_ARCHIVE_H)
#define OTTER_TRACE_ARCHIVE_H

#include <stdbool.h>
#include "otter/otter-common.h"

/* interface function prototypes */
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

#endif // OTTER_TRACE_ARCHIVE_H
