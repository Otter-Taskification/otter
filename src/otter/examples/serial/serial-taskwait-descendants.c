#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <otter/otter-serial.h>
#define LEN 5

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s [0|1]\n", argv[0]);
        return 1;
    }

    otter_task_sync_t sync_mode = atoi(argv[1]);

    otterTraceInitialise(OTTER_SRC_ARGS());
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterTaskBegin(OTTER_SRC_ARGS());
        for (int k=0; k<LEN; k++)
        {
            otterTaskBegin(OTTER_SRC_ARGS());
            otterTaskEnd();
        }
        otterTaskEnd();

        otterSynchroniseTasks(sync_mode);

    }
    otterThreadsEnd();
    otterTraceFinalise();

    return 0;
}
