#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "api/otter-task-graph/otter-task-graph-wrapper.hpp"

using namespace otter;

// Have to modify app code to accept otter task handles if tasks are nested :(
int fib(int n, Task& parent);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    auto& root = Otter::get_otter().get_root_task();

    #pragma omp parallel shared(fibn, n, root)
    {
        #pragma omp single
        fibn = fib(n, root);
    }

    root.synchronise_tasks(otter_sync_children);
    printf("f(%d) = %d\n", n, fibn);
    return 0;
}

int fib(int n, Task& parent) {
    auto task = parent.make_child();

    if (n<2) {
        return n;
    }
    int i, j;

    // Tag: wrap a task
    #pragma omp task shared(i) shared(task) firstprivate(n)
    i = fib(n-1, task);

    // Tag: wrap a task
    #pragma omp task shared(j) shared(task) firstprivate(n)
    j = fib(n-2, task);
    
    // Indicate a synchronisation constraint on a subset of work items
    // Must appear AFTER the taskwait itself
    #pragma omp taskwait
    task.synchronise_tasks(otter_sync_children);

    return i+j;
}
