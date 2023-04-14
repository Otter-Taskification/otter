#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "public/debug.h"
#include "public/otter-trace/trace-initialise.h"
#include "otter-trace/trace-static-constants.h"
#include "otter-trace/trace-archive.h"

enum { char_buff_sz = 1024 };

/**
 * @brief Copy the process' memory map from /proc/self/maps to aux/maps within
 * the trace directory. This information can be used to match return addresses
 * to source locations.
 * 
 * @param opt The runtime options passed to Otter.
 */
static void trace_copy_proc_maps(otter_opt_t *opt);

// TODO: accept injected state
bool trace_initialise(otter_opt_t *opt)
{
    // Determine the archive name from the options

    static char archive_name[default_name_buf_sz+1] = {0};
    char *p = &archive_name[0];

    /* Copy filename */
    strncpy(p, opt->tracename, default_name_buf_sz - strlen(archive_name));
    p = &archive_name[0] + strlen(archive_name);

    /* Copy hostname */
    if (opt->append_hostname)
    {
        strncpy(p, ".", default_name_buf_sz - strlen(archive_name));
        p = &archive_name[0] + strlen(archive_name);
        strncpy(p, opt->hostname, default_name_buf_sz - strlen(archive_name));
        p = &archive_name[0] + strlen(archive_name);
    }

    /* Copy PID */
    strncpy(p, ".", default_name_buf_sz - strlen(archive_name));
    p = &archive_name[0] + strlen(archive_name);
    snprintf(p, default_name_buf_sz - strlen(archive_name), "%u", getpid());
    p = &archive_name[0] + strlen(archive_name);

    /* Copy path + filename */
    char archive_path[default_name_buf_sz+1] = {0};
    snprintf(archive_path, default_name_buf_sz, "%s/%s", opt->tracepath, archive_name);

    fprintf(stderr, "%-30s %s/%s\n",
        "Trace output path:", opt->tracepath, archive_name);

    /* Store archive name in options struct */
    opt->archive_name = &archive_name[0];

    trace_copy_proc_maps(opt);

    // TODO: pass state here
    return trace_initialise_archive(&archive_path[0], opt->archive_name, opt->event_model);
}

static void
trace_copy_proc_maps(otter_opt_t *opt)
{
    char     oname[char_buff_sz] = {0};
    FILE    *ifile               = NULL;
    FILE    *ofile               = NULL;
    char    *linebuff            = NULL;
    size_t   linesize            = 0;

    // create aux files dir
    snprintf(oname, char_buff_sz, "%s/%s/aux", opt->tracepath, opt->archive_name);
    if (mkdir(oname, 0755) == -1) {
        LOG_ERROR("(line %d) Error while making dir %s: %s", __LINE__, oname, strerror(errno));
        goto exit_error;
    } else {
        LOG_DEBUG("created dir: %s", oname);
    }

    // open maps file
    if ((ifile = fopen("/proc/self/maps", "r")) == NULL) {
        LOG_ERROR("(line %d) Error opening file /proc/self/maps: %s", __LINE__, strerror(errno));
        goto exit_error;
    }

    // open output
    snprintf(oname, char_buff_sz, "%s/%s/aux/maps", opt->tracepath, opt->archive_name);
    if ((ofile = fopen(oname, "w")) == NULL) {
        LOG_ERROR("(line %d) Error opening file %s: %s", __LINE__, oname, strerror(errno));
        goto exit_error;
    }

    // copy ifile -> ofile
    while (getline(&linebuff, &linesize, ifile) > 0) {
        size_t linelen = strlen(linebuff);
        linebuff[linelen-1] = '\0'; // replace delimiting newline with null-byte for logging
        LOG_DEBUG("/proc/self/maps: %s", linebuff);
        if (fprintf(ofile, "%s\n", linebuff) < 0) {
            LOG_ERROR("(line %d) Error writing to file %s: %s", __LINE__, oname, strerror(errno));
            goto exit_error;
        }
    }

    LOG_DEBUG("copied memory map to %s", oname);

exit_error:
    if(ifile)      fclose(ifile);
    if(ofile)      fclose(ofile);
    if(linebuff)   free(linebuff);
    return;
}

// TODO: accept injected state
bool trace_finalise(void)
{
    // TODO: pass state here
    return trace_finalise_archive();
}
