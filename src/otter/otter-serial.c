#include <stdio.h>

#include "otter/otter-version.h"
#include "otter/general.h"
#include "otter/debug.h"
#include "otter/otter-environment-variables.h"
#include "otter/trace.h"
#include "otter/otter-serial.h"

void otterTraceBegin(void)
{
    fprintf(stderr, "%s\n", __func__);
    return;
}

void otterTraceEnd(void)
{
    fprintf(stderr, "%s\n", __func__);
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
