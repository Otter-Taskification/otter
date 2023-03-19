#include <stdio.h>
#include <stdlib.h>
#include <otter/otter-task-graph.h>

// Have to modify app code to accept otter task handles if tasks are nested :(
int fib(int n, otter_task_context *parent);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    otterTraceInitialise();

    otter_task_context *root = otterTaskBegin(OTTER_SRC_ARGS(), NULL);
    fibn = fib(n, root);
    otterTaskEnd(root);

    printf("f(%d) = %d\n", n, fibn);

    otterTraceFinalise();

    return 0;
}

int fib(int n, otter_task_context *parent) {
    if (n<2) return n;
    int i, j;

    // Tag: wrap a task
    otter_task_context *child = otterTaskBegin(OTTER_SRC_ARGS(), parent);
    i = fib(n-1, child);
    otterTaskEnd(child);

    // Tag: wrap a task
    child = otterTaskBegin(OTTER_SRC_ARGS(), parent);
        j = fib(n-2, child);
    otterTaskEnd(child);

    // Indicate a synchronisation constraint on a subset of work items
    // otterSynchroniseTasks(otter_sync_children);
    otterSynchroniseTasks(parent, otter_sync_children);

    return i+j;
}
