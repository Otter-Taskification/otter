#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>

int main(void)
{
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
