#include "api/otter-serial/otter-serial.h"
#include <stdio.h>
#include <unistd.h>
#define LEN 5

/**
 * @brief Assert that pyotter works as expected with single regions.
 */

int main(void) {
  int j = 0;
  otterTraceInitialise(OTTER_SRC_ARGS());
  otterThreadsBegin(OTTER_SRC_ARGS());
  {
    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();
    otterSynchroniseTasks(otter_sync_children);

    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();

    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();

    otterSynchroniseTasks(otter_sync_children);

    // This task is not synchronised by the taskwait above
    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();

    otterSynchroniseTasks(otter_sync_children);

    // This task is not synchronised by the taskwait above
    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();

    otterSynchroniseTasks(otter_sync_children);

    // This task is not synchronised by the taskwait above
    otterTaskBegin(OTTER_SRC_ARGS());
    otterTaskEnd();
  }
  otterThreadsEnd();
  otterTraceFinalise();

  return 0;
}
