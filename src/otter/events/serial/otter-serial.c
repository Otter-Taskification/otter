#define __USE_POSIX // HOST_NAME_MAX
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "otter/otter-version.h"
#include "otter/general.h"
#include "otter/debug.h"
#include "otter/otter-environment-variables.h"
#include "otter/trace.h"
#include "otter/trace-archive.h"
#include "otter/otter-serial.h"



void otterTraceBegin(void)
{
    // Initialise archive

    fprintf(stderr, "%s\n", __func__);

    /* detect environment variables */
    static otter_opt_t opt = {
        .hostname         = NULL,
        .tracename        = NULL,
        .tracepath        = NULL,
        .archive_name     = NULL,
        .append_hostname  = false
    };

    static char host[HOST_NAME_MAX+1] = {0};
    gethostname(host, HOST_NAME_MAX);

    opt.hostname = host;
    opt.tracename = getenv(ENV_VAR_TRACE_OUTPUT);
    opt.tracepath = getenv(ENV_VAR_TRACE_PATH);
    opt.append_hostname = getenv(ENV_VAR_APPEND_HOST) == NULL ? false : true;

    /* Apply defaults if variables not provided */
    if(opt.tracename == NULL) opt.tracename = DEFAULT_OTF2_TRACE_OUTPUT;
    if(opt.tracepath == NULL) opt.tracepath = DEFAULT_OTF2_TRACE_PATH;

    LOG_INFO("Otter environment variables:");
    LOG_INFO("%-30s %s", "host", opt.hostname);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_PATH,   opt.tracepath);
    LOG_INFO("%-30s %s", ENV_VAR_TRACE_OUTPUT, opt.tracename);
    LOG_INFO("%-30s %s", ENV_VAR_APPEND_HOST,  opt.append_hostname?"Yes":"No");

    trace_initialise_archive(&opt);

    return;
}

void otterTraceEnd(void)
{
    // Finalise arhchive
    fprintf(stderr, "%s\n", __func__);
    trace_finalise_archive();
    return;
}

void otterParallelBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterParallelEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskBeginSingle(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTaskEndSingle(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopIterationBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterLoopIterationEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterSynchroniseChildTasks(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterSynchroniseDescendantTasks(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}
