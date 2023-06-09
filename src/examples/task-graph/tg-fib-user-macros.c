#include <stdio.h>
#include <stdlib.h>
#define OTTER_TASK_GRAPH_ENABLE_USER
#include "api/otter-task-graph/otter-task-graph-user.h"

static const char* fib_format = "fib(%d)";

int fib(int n);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    OTTER_INITIALISE();

    char phase[256] = {0};
    snprintf(&phase[0], 255, "calculate fib(%d)", n);

    OTTER_DECLARE_HANDLE(root);
    OTTER_INIT_TASK(root, OTTER_NULL_TASK, 0, otter_add_to_pool, fib_format, n);

    OTTER_PHASE_BEGIN(phase);
    OTTER_TASK_START(root);
    fibn = fib(n);
    OTTER_TASK_END(root);
    OTTER_PHASE_END();

    OTTER_TASK_WAIT_FOR(OTTER_NULL_TASK, children);

    printf("fib(%d) = %d\n", n, fibn);

    OTTER_FINALISE();

    printf("%s\n", phase);
    return 0;
}


int fib(int n) {
    if (n<2) return n;
    int i, j;

    OTTER_DECLARE_HANDLE(parent);
    OTTER_REMOVE_FROM_POOL(parent, fib_format, n);

    OTTER_DECLARE_HANDLE(child1);
    OTTER_INIT_TASK(child1, parent, 0, otter_add_to_pool, fib_format, n-1);
    OTTER_TASK_START(child1);
    i = fib(n-1);
    OTTER_TASK_END(child1);

    OTTER_DECLARE_HANDLE(child2);
    OTTER_INIT_TASK(child2, parent, 0, otter_add_to_pool, fib_format, n-2);
    OTTER_TASK_START(child2);
    j = fib(n-2);
    OTTER_TASK_END(child2);

    OTTER_TASK_WAIT_FOR(parent, children);

    return i+j;
}
