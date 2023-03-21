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

    #pragma omp parallel shared(fibn, n)
    {
        #pragma omp single
        {
            otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);
            fibn = fib(n, root);
            otterTaskEnd(root);
        }
    }

    printf("f(%d) = %d\n", n, fibn);

    otterTraceFinalise();

    return 0;
}

int fib(int n, otter_task_context *encountering_task) {

    if (n<2) {
        return n;
    }
    int i, j;

    // Tag: wrap a task
    otter_task_context *child_1 = otterTaskBegin(OTTER_SRC_ARGS(), encountering_task);
    #pragma omp task shared(i, child_1) firstprivate(n)
    {
        i = fib(n-1, child_1);
        // End the task *INSIDE* the task body to prevent use-after-free due to destroying the task before the OMP task is actually executed
        otterTaskEnd(child_1);
    }

    // Tag: wrap a task
    otter_task_context *child_2 = otterTaskBegin(OTTER_SRC_ARGS(), encountering_task);
    #pragma omp task shared(j, child_2) firstprivate(n)
    {
        j = fib(n-2, child_2);
        otterTaskEnd(child_2);
    }
    
    // Indicate a synchronisation constraint on a subset of work items
    // Must appear AFTER the taskwait itself
    #pragma omp taskwait
    otterSynchroniseTasks(encountering_task, otter_sync_children);

    return i+j;
}
