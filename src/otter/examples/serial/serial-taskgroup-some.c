#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(void)
{
    otterTraceInitialise();
    otterParallelBegin();
    {        
        // This task is not synchronised in the group below
        otterTaskBegin();
        otterTaskEnd();

        otterSynchroniseDescendantTasksBegin();

        otterTaskBegin();
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin();
            otterTaskEnd();
        }
        otterTaskEnd();

        otterTaskBegin();
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin();
            otterTaskEnd();
        }
        otterTaskEnd();

        otterTaskBegin();
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin();
            otterTaskEnd();
        }
        otterTaskEnd();
        
        otterSynchroniseDescendantTasksEnd();
        
        // This task is not synchronised in the group above
        otterTaskBegin();
        otterTaskEnd();

    }
    otterParallelEnd();
    otterTraceFinalise();

    return 0;
}
