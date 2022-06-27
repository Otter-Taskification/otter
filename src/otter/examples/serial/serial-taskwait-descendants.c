#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(void)
{
    otterTraceInitialise();
    otterParallelBegin();
    {
        otterTaskBegin();
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin();
            otterTaskEnd();
        }
        otterTaskEnd();

        otterSynchroniseChildTasks();

    }
    otterParallelEnd();
    otterTraceFinalise();

    return 0;
}
