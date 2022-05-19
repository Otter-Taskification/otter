#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(void)
{
    int j=0;
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterTaskSingleBegin();
        {
            otterLoopBegin();
            for (j=0; j<LEN; j++)
            {
                otterTaskBegin(OTTER_SRC_ARGS());
                usleep(50);
                otterTaskEnd();
            }
            otterLoopEnd();
        }
        otterTaskSingleEnd();
        otterSynchroniseChildTasks();

        otterSynchroniseDescendantTasksBegin();
        otterLoopBegin();
        for (j=0; j<LEN; j++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
                otterTaskBegin(OTTER_SRC_ARGS());
                usleep(50);
                otterTaskEnd();
                otterTaskBegin(OTTER_SRC_ARGS());
                usleep(50);
                otterTaskEnd();
                otterTaskBegin(OTTER_SRC_ARGS());
                usleep(50);
                otterTaskEnd();
            otterTaskEnd();
        }
        otterLoopEnd();
        otterSynchroniseDescendantTasksEnd();

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
