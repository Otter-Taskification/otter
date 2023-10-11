#include <omp.h>
#include <stdio.h>
#include <unistd.h>

static const int N = 1;

void taskloop_nogroup(void);
void taskgroup(void);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s mode\n", argv[0]);
    return 1;
  }

  int mode = atoi(argv[1]);
  int result = 0;

#pragma omp parallel firstprivate(mode)
  {
    if (mode == 1) {
      taskloop_nogroup();
      taskgroup();
    } else if (mode == 2) {
      taskgroup();
      taskloop_nogroup();
    } else {
      fprintf(stderr, "illegal value: %d\n", mode);
#pragma omp single
      result = 1;
    }
  }
  return result;
}

void taskloop_nogroup(void) {
#pragma omp taskloop nogroup
  for (int k = 0; k < N; k++) {
    usleep(50);
  }
#pragma omp taskwait
  return;
}

void taskgroup(void) {
#pragma omp taskgroup
  {
#pragma omp single
    {
      for (int k = 0; k < N; k++) {
#pragma omp task
        usleep(50);
      }
    }
  }
  return;
}
