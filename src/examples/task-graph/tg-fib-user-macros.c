#include <stdio.h>
#include <stdlib.h>
#define OTTER_TASK_GRAPH_ENABLE_USER
#include "api/otter-task-graph/otter-task-graph-user.h"

int fib(int n);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int fibn = 0;

    OTTER_INITIALISE();

    OTTER_TASK_GRAPH_DECLARE_TASK(root);
    OTTER_TASK_GRAPH_INIT_TASK(root, NULL, 0, NULL, false);
    OTTER_TASK_GRAPH_REGISTER_TASK(root, "fib(%d)", n);

    OTTER_TASK_GRAPH_START_TASK(root, 0);
    fibn = fib(n);
    OTTER_TASK_GRAPH_FINISH_TASK(root);

    printf("f(%d) = %d\n", n, fibn);

    OTTER_FINALISE();

    return 0;
}

#define FIB_GET_PARENT 1

int fib(int n) {
    if (n<2) return n;
    int i, j;

#if FIB_GET_PARENT
    OTTER_TASK_GRAPH_DECLARE_TASK(parent);
    OTTER_TASK_GRAPH_GET_TASK(parent, "fib(%d)", n); // CAUTION: there might be multiple tasks with the same label!
    /**
     * fib(n)
     *  |
     *  |- fib(n-1)
     *  |   |
     *  |   |- fib(n-2) <-- registers label "fib(n-2)"
     *  |   |
     *  |   \- fib(n-3)
     *  |
     *  \- fib(n-2) <-- registers label "fib(n-2)"
     *      |
     *      |- fib(n-3)
     *      |
     *      \- fib(n-4)
     * 
     */
#endif

    OTTER_TASK_GRAPH_DECLARE_TASK(child1);
    OTTER_TASK_GRAPH_INIT_TASK(child1, NULL, 0, NULL, false);
    OTTER_TASK_GRAPH_REGISTER_TASK(child1, "fib(%d)", n-1);

    OTTER_TASK_GRAPH_START_TASK(child1, 0);
    i = fib(n-1);
    OTTER_TASK_GRAPH_FINISH_TASK(child1); // deletes child

    OTTER_TASK_GRAPH_DECLARE_TASK(child2);
    OTTER_TASK_GRAPH_INIT_TASK(child2, NULL, 0, NULL, false);
    OTTER_TASK_GRAPH_REGISTER_TASK(child2, "fib(%d)", n-2);

    OTTER_TASK_GRAPH_START_TASK(child2, 0);
        j = fib(n-2);
    OTTER_TASK_GRAPH_FINISH_TASK(child2);

#if FIB_GET_PARENT
    OTTER_TASK_GRAPH_SYNCHRONISE_TASKS(parent, children);
#endif

    return i+j;
}
