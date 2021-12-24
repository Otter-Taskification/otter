#include <stdio.h>
#include <unistd.h>
#include "otter/otter-serial.h"
#define LEN 5

int main(void)
{
    int j=0;
    otterTraceInitialise();
    otterParallelBegin();
    {
        otterTaskSingleBegin();
        {
            otterLoopBegin();
            for (j=0; j<LEN; j++)
            {
                otterTaskBegin();
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
            otterTaskBegin();
                otterTaskBegin();
                usleep(50);
                otterTaskEnd();
                otterTaskBegin();
                usleep(50);
                otterTaskEnd();
                otterTaskBegin();
                usleep(50);
                otterTaskEnd();
            otterTaskEnd();
        }
        otterLoopEnd();
        otterSynchroniseDescendantTasksEnd();

    }
    otterParallelEnd();
    otterTraceFinalise();

    return 0;
}
