#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(void)
{
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterTaskBegin(OTTER_SRC_ARGS());
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
            otterTaskEnd();
        }
        otterTaskEnd();

        otterSynchroniseTasks(otter_sync_children);

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
