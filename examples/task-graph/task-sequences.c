#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define OTTER_TASK_GRAPH_ENABLE_USER
#include "api/otter-task-graph/otter-task-graph-user.h"

void spawn_children_and_wait(bool with_barrier, const char *parent_label);

int main(int argc, char *argv[]) {
  OTTER_INITIALISE();
  OTTER_PHASE_SWITCH("calculate");
  for (int step = 0; step < 3; step++) {
    spawn_children_and_wait(true, NULL);
  }
  OTTER_DEFINE_TASK(parent, OTTER_NULL_TASK, otter_add_to_pool, "parent task");
  OTTER_TASK_START(parent);
  spawn_children_and_wait(false, "parent task");
  OTTER_TASK_END(parent);
  OTTER_TASK_WAIT_FOR(OTTER_NULL_TASK, descendants);
  OTTER_FINALISE();
  return 0;
}

void spawn_children_and_wait(bool with_barrier, const char *parent_label) {
  OTTER_DECLARE_HANDLE(parent);
  if (parent_label != NULL) {
    OTTER_POOL_BORROW(parent, parent_label);
  }
  for (int k = 0; k < 5; k++) {
    OTTER_DEFINE_TASK(task, parent, otter_no_add_to_pool, "standard task");
    OTTER_TASK_START(task);
    usleep(100);
    OTTER_TASK_END(task);
  }
  if (with_barrier) {
    OTTER_TASK_WAIT_FOR(parent, children);
  }
}
