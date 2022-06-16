#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

/**
 * @brief Assert that pyotter works as expected with single regions.
 */

int main(void)
{
    int j=0;
    otterTraceInitialise();
    otterParallelBegin();
    {
        otterTaskBegin(); otterTaskEnd();
        otterSynchroniseChildTasks();

        otterTaskBegin(); otterTaskEnd();

        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();

    }
    otterParallelEnd();
    otterTraceFinalise();

    return 0;
}
