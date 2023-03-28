#include "public/otter-trace/trace-mmap.h"

#define CHAR_BUFF_SZ 1024

void trace_copy_proc_maps(otter_opt_t *opt) {
    char     oname[CHAR_BUFF_SZ] = {0};
    FILE    *ifile               = NULL;
    FILE    *ofile               = NULL;
    char    *linebuff            = NULL;
    size_t   linesize            = 0;

    // TODO: consider separating concerns of setting up dirs and copying files
    // TODO: as setting up environment should be done during initialisation.
    // create aux files dir
    snprintf(oname, CHAR_BUFF_SZ, "%s/%s/aux", opt->tracepath, opt->archive_name);
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
    snprintf(oname, CHAR_BUFF_SZ, "%s/%s/aux/maps", opt->tracepath, opt->archive_name);
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
