#include "api/otter-serial/otter-serial.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  otterTraceInitialise(OTTER_SRC_ARGS());
  otterThreadsBegin(OTTER_SRC_ARGS());
  otterTaskBegin(OTTER_SRC_ARGS());
  otterTaskEnd();
  otterThreadsEnd();
  otterTraceFinalise();

  return 0;
}
