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
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();
        
        otterSynchroniseChildTasks();
        
        // This task is not synchronised by the taskwait above
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
