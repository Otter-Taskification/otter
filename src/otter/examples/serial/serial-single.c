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
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterTaskBegin(OTTER_SRC_ARGS()); otterTaskEnd();
        otterSynchroniseTasks(otter_sync_children);

        otterTaskBegin(OTTER_SRC_ARGS()); otterTaskEnd();

        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseTasks(otter_sync_children);
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseTasks(otter_sync_children);
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();
        
        otterSynchroniseTasks(otter_sync_children);
        
        // This task is not synchronised by the taskwait above
        otterTaskSingleBegin();
        otterTaskSingleEnd();

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
