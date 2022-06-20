#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

/**
 * @brief Assert that pyotter produces the expected graph when not all child
 * tasks are synchronised by a taskwait barrier.
 */

int main(void)
{
    int j=0;
    otterTraceInitialise();
    otterParallelBegin();
    {
        otterTaskBegin();
        otterTaskEnd();

        otterTaskBegin();
        otterTaskEnd();

        otterTaskBegin();
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin();
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin();
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin();
        otterTaskEnd();

    }
    otterParallelEnd();
    otterTraceFinalise();

    return 0;
}
