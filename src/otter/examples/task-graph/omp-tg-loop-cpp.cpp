#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <otter/otter-task-graph-wrapper.hpp>

using namespace otter;

// Have to modify app code to accept otter task handles if tasks are nested :(
void spawn_tasks_in_loop(int num_tasks, Task& task);

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "usage: %s tasks loops\n", argv[0]);
        return 1;
    }

    int tasks_per_loop = atoi(argv[1]);
    int loops = atoi(argv[2]);

    auto& root = Otter::get_otter().get_root_task();

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

    return 0;
}

void spawn_tasks_in_loop(int num_tasks, Task& task)
{
    #pragma omp taskloop nogroup grainsize(1) shared(task)
    for (int n=0; n<num_tasks; n++) {
        Task child = task.make_child();
    }
    #pragma omp taskwait
    task.synchronise_tasks(otter_sync_children);
}
