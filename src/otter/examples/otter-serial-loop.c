#include <stdio.h>
#include <unistd.h>
#include "otter/otter-serial.h"

#define LEN 5

int main(void)
{
    int j=0;
    otterTraceBegin();
    otterParallelBegin();
    {
        otterTaskBeginSingle();
        {
            otterLoopBegin();
            for (j=0; j<LEN; j++)
            {
                otterTaskBegin();
                usleep(50);
                otterTaskEnd();
            }
            otterLoopEnd();
            otterSynchroniseChildTasks();
        }
        otterTaskEndSingle();

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
        otterSynchroniseChildTasks();

    }
    otterParallelEnd();
    otterTraceEnd();

    return 0;
}
