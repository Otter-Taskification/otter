#include <stdio.h>
#include <stdlib.h>
#include "otter/otter-serial.h"

int fib(int n);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    otterTraceBegin();

    // Tag: start of a region we want to parallelise
    otterParallelBegin();
    {
        // Tag: wrap a task
        otterTaskSingleBegin();
            fibn = fib(n);
        otterTaskSingleEnd();
    }
    // Tag: end of a region we want to parallelise
    otterParallelEnd();

    printf("f(%d) = %d\n", n, fibn);

    otterTraceEnd();

    return 0;
}

int fib(int n) {
    if (n<2) return n;
    int i, j;

    // Tag: wrap a task
    otterTaskBegin();
        i = fib(n-1);
    otterTaskEnd();

    // Tag: wrap a task
    otterTaskBegin();
        j = fib(n-2);
    otterTaskEnd();

    // Indicate a synchronisation constraint on a subset of work items
    otterSynchroniseChildTasks();

    return i+j;
}
