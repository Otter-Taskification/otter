#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "public/debug.h"
#include "public/otter-common.h"

/**
 * @brief Copy the process' memory map from /proc/self/maps to aux/maps within
 * the trace directory. This information can be used to match return addresses
 * to source locations.
 * 
 * @param opt The runtime options passed to Otter.
 */
void trace_copy_proc_maps(otter_opt_t *opt);
