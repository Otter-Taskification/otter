#if !defined(OTTER_TRACE_ARCHIVE_H)
#define OTTER_TRACE_ARCHIVE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"

#define DEFAULT_LOCATION_GRP 0 // OTF2_UNDEFINED_LOCATION_GROUP
#define DEFAULT_SYSTEM_TREE  0
#define DEFAULT_NAME_BUF_SZ  256

/* interface function prototypes */
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

#endif // OTTER_TRACE_ARCHIVE_H
