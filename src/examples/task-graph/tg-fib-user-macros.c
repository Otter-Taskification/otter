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

    OTTER_DECLARE(root);
    OTTER_INIT(root, NULL, 0, true, fib_format, n);
    OTTER_TASK_START(root);
    fibn = fib(n);
    OTTER_TASK_END(root);

    printf("fib(%d) = %d\n", n, fibn);

    OTTER_FINALISE();

    return 0;
}


int fib(int n) {
    if (n<2) return n;
    int i, j;

    OTTER_DECLARE(parent);
    OTTER_POP(parent, fib_format, n);

    OTTER_DECLARE(child1);
    OTTER_INIT(child1, parent, 0, true, fib_format, n-1);
    OTTER_TASK_START(child1);
    i = fib(n-1);
    OTTER_TASK_END(child1);

    OTTER_DECLARE(child2);
    OTTER_INIT(child2, parent, 0, true, fib_format, n-2);
    OTTER_TASK_START(child2);
    j = fib(n-2);
    OTTER_TASK_END(child2);

    OTTER_SYNCHRONISE(parent, children);

    return i+j;
}
