#include "api/otter-serial/otter-serial.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
  otterTraceInitialise(OTTER_SRC_ARGS());
  otterPhaseBegin("MAIN");
  otterThreadsBegin(OTTER_SRC_ARGS());
  {
    otterPhaseBegin("compute");
    otterPhaseSwitch("communicate");
    otterPhaseEnd();
  }
  otterThreadsEnd();
  otterPhaseEnd();
  otterTraceFinalise();

  return 0;
}
