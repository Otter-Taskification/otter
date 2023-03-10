#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <otter/otter-serial.h>

void spawn_tasks_recursive(int tasks, int level)
{
    if (level <= 0) {
        return;
    }
    int num_spawn = tasks;
    while(num_spawn--) {
        otterTaskBegin(OTTER_SRC_ARGS());
        spawn_tasks_recursive(tasks, level-1);
        otterTaskEnd();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s levels\n", argv[0]);
        return 1;
    }

    int levels = atoi(argv[1]);
    otterTraceInitialise(OTTER_SRC_ARGS());
    otterPhaseBegin("=== MAIN ===");
    otterThreadsBegin(OTTER_SRC_ARGS());
    {
        otterPhaseBegin("compute");

        spawn_tasks_recursive(2, levels);
        otterSynchroniseTasks(otter_sync_descendants);

        otterPhaseSwitch("communicate");

        spawn_tasks_recursive(1, levels+1);
        otterSynchroniseTasks(otter_sync_children);

        otterPhaseEnd();
    }
    otterThreadsEnd();
    otterPhaseEnd();
    otterTraceFinalise();

    return 0;
}
