#include <omp.h>
#include <stdio.h>
#define OTTER_TASK_GRAPH_ENABLE_USER
#include "api/otter-task-graph/otter-task-graph-user.h"

int main(void) {

  OTTER_INITIALISE();

  OTTER_DECLARE_HANDLE(task);

#pragma omp parallel
  {
    OTTER_INIT_TASK(task, OTTER_NULL_TASK, otter_no_add_to_pool, "task");
    fprintf(stderr, "%d\n", omp_get_thread_num());
    OTTER_TASK_START(task);
    OTTER_TASK_END(task);
  }

  OTTER_FINALISE();

  return 0;
}
