#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#ifdef Use_Otter
#include "api/otter-task-graph/otter-task-graph.h"
#else
#include "api/otter-task-graph/otter-task-graph-stub.h"
#endif

// Have to modify app code to accept otter task handles if tasks are nested :(
void spawn_tasks_in_loop(int num_tasks, otter_task_context *task);

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "usage: %s tasks loops\n", argv[0]);
        return 1;
    }

    int tasks_per_loop = atoi(argv[1]);
    int loops = atoi(argv[2]);

    otterTraceInitialise();

    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int loop=0; loop<loops; loop++) {
                fprintf(stderr, "[%3d/%3d]: spawning %d tasks\n", loop+1, loops, tasks_per_loop);
                spawn_tasks_in_loop(tasks_per_loop, root);
            }
        }
    }
    otterTaskEnd(root);

    otterTraceFinalise();

    return 0;
}

void spawn_tasks_in_loop(int num_tasks, otter_task_context *task)
{
    #pragma omp taskloop nogroup grainsize(1)
    for (int n=0; n<num_tasks; n++) {
        usleep(1);
    }
    #pragma omp taskwait
    otterSynchroniseTasks(task, otter_sync_children);
}
