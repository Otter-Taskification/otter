#include "api/otter-serial/otter-serial.h"
#include <stdio.h>
#include <unistd.h>
#define LEN 5

int main(void) {
  int j = 0;
  otterTraceInitialise(OTTER_SRC_ARGS());
  otterThreadsBegin(OTTER_SRC_ARGS());
  {
    otterTaskBegin(OTTER_SRC_ARGS());
    {
      otterLoopBegin();
      for (j = 0; j < LEN; j++) {
        otterTaskBegin(OTTER_SRC_ARGS());
        usleep(50);
        otterTaskEnd();
      }
      otterLoopEnd();
    }
    otterTaskEnd();
    otterSynchroniseTasks(otter_sync_children);

    otterSynchroniseDescendantTasksBegin();
    otterLoopBegin();
    for (j = 0; j < LEN; j++) {
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
