#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <otter/otter-task-graph.h>

// Have to modify app code to accept otter task handles if tasks are nested :(
int fib(int n, otter_task_context *task);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    otterTraceInitialise();

    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);

    #pragma omp parallel shared(fibn, n)
    {
        #pragma omp single
        fibn = fib(n, root);
    }

    otterSynchroniseTasks(root, otter_sync_children);

    otterTaskEnd(root);
    printf("f(%d) = %d\n", n, fibn);
    otterTraceFinalise();

    return 0;
}

int fib(int n, otter_task_context *parent) {
    otter_task_context *task = otterTaskBegin(OTTER_SRC_ARGS(), parent);

    if (n<2) {
        otterTaskEnd(task);
        return n;
    }
    int i, j;

    // Tag: wrap a task
    #pragma omp task shared(i) firstprivate(n)
    i = fib(n-1, task);

    // Tag: wrap a task
    #pragma omp task shared(j) firstprivate(n)
    j = fib(n-2, task);
    
    // Indicate a synchronisation constraint on a subset of work items
    // Must appear AFTER the taskwait itself
    #pragma omp taskwait
    otterSynchroniseTasks(task, otter_sync_children);
    
    otterTaskEnd(task);

    return i+j;
}
