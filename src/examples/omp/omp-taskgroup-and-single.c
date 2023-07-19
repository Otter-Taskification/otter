#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void taskgroup_inside_single(void);
void taskgroup_outside_single(void);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s mode\n", argv[0]);
    return 1;
  }

  int mode = atoi(argv[1]);

  if (mode == 1) {
    taskgroup_inside_single();
  } else if (mode == 2) {
    taskgroup_outside_single();
  } else {
    fprintf(stderr, "illegal value: %d\n", mode);
    return 1;
  }

  return 0;
}

void taskgroup_inside_single(void) {
#pragma omp parallel
  {
#pragma omp single
    {
#pragma omp taskgroup
      {
#pragma omp task
        { usleep(50); }
      }
    }
  }
  return;
}

void taskgroup_outside_single(void) {
#pragma omp parallel
  {
#pragma omp taskgroup
    {
#pragma omp single
      {
#pragma omp task
        { usleep(50); }
      }
    }
  }
  return;
}
