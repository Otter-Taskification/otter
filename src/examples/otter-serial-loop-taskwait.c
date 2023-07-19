#include "api/otter-serial/otter-serial.h"

static const int LEN = 5;

int main(void) {
  otterTraceInitialise(OTTER_SRC_ARGS());
  otterThreadsBegin(OTTER_SRC_ARGS());
  {
    otterLoopBegin();
    for (int i = 0; i < LEN; i++) {
      otterTaskBegin(OTTER_SRC_ARGS());
      {
        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();

        otterTaskBegin(OTTER_SRC_ARGS());
        otterTaskEnd();
      }
      otterTaskEnd();
    }
    otterLoopEnd();

    otterLoopBegin();
    for (int i = 0; i < LEN; i++) {
      otterTaskBegin(OTTER_SRC_ARGS());
      otterTaskEnd();
    }
    otterLoopEnd();

    otterSynchroniseTasks(otter_sync_descendants);

    otterLoopBegin();
    for (int i = 0; i < LEN; i++) {
      otterTaskBegin(OTTER_SRC_ARGS());
      otterTaskEnd();
    }
    otterLoopEnd();

    otterSynchroniseTasks(otter_sync_children);
  }
  otterThreadsEnd();
  otterTraceFinalise();

  return 0;
}
