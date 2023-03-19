#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(void)
{
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {        
        // This task is not synchronised in the group below
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

        otterSynchroniseDescendantTasksBegin();

        otterTaskBegin(OTTER_SRC_ARGS());
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
            otterTaskEnd();
        }
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
            otterTaskEnd();
        }
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
            otterTaskEnd();
        }
        otterTaskEnd();
        
        otterSynchroniseDescendantTasksEnd();
        
        // This task is not synchronised in the group above
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
