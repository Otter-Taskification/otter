#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "api/otter-task-graph/otter-task-graph.h"

void spawn_tasks_recursive(int tasks, int level, otter_task_context *parent)
{
    if (level <= 0) {
        return;
    }
    int num_spawn = tasks;
    otter_task_context *task = NULL;
    while(num_spawn--) {
        task = otterTaskBegin(OTTER_SRC_ARGS(), parent);
        spawn_tasks_recursive(tasks, level-1, task);
        otterTaskEnd(task);
    }
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s levels\n", argv[0]);
        return 1;
    }

    int levels = atoi(argv[1]);
    otterTraceInitialise();
    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);
    {
        otter_task_context *child = otterTaskBegin(OTTER_SRC_ARGS(), root);
        spawn_tasks_recursive(2, levels, child);
        otterTaskEnd(child);
        otterSynchroniseTasks(root, otter_sync_descendants);

        child = otterTaskBegin(OTTER_SRC_ARGS(), root);
        spawn_tasks_recursive(1, levels+1, child);
        otterTaskEnd(child);
        otterSynchroniseTasks(root, otter_sync_children);

    }
    otterTaskEnd(root);
    otterTraceFinalise();

    return 0;
}
