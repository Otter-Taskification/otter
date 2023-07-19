#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#define OTTER_TASK_GRAPH_ENABLE_USER
#include "api/otter-task-graph/otter-task-graph-user.h"

void spawn_children_and_wait(bool with_barrier, const char* parent_label);

int main(int argc, char *argv[]) {
    OTTER_INITIALISE();
    OTTER_DECLARE_HANDLE(parent);
    OTTER_DECLARE_HANDLE(child);
    OTTER_PHASE_SWITCH("calculate");
    for (int step=0; step<3; step++) {
        spawn_children_and_wait(true, NULL);
    }
    OTTER_INIT_TASK(parent, OTTER_NULL_TASK, 2, otter_add_to_pool, "parent task");
    OTTER_TASK_START(parent);
    spawn_children_and_wait(false, "parent task");
    // OTTER_TASK_WAIT_FOR(parent, descendants);
    OTTER_TASK_END(parent);
    OTTER_TASK_WAIT_FOR(OTTER_NULL_TASK, descendants);
    OTTER_FINALISE();
    return 0;
}

void spawn_children_and_wait(bool with_barrier, const char* parent_label) {
    OTTER_DECLARE_HANDLE(parent);
    OTTER_DECLARE_HANDLE(task);
    if (parent_label != NULL) {
        OTTER_BORROW_FROM_POOL(parent, parent_label);
    }
    for (int k=0; k<5; k++) {
        OTTER_INIT_TASK(task, parent, 1, otter_no_add_to_pool, "standard task");
        OTTER_TASK_START(task);
        usleep(100);
        OTTER_TASK_END(task);
    }
    if (with_barrier) {
        OTTER_TASK_WAIT_FOR(parent, children);
    }
}
