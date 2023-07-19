#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s threads tasks\n", argv[0]);
    return 1;
  }

  int threads = atoi(argv[1]);
  int tasks = atoi(argv[2]);

  omp_set_max_active_levels(2);

#pragma omp parallel num_threads(threads)
  {
#pragma omp taskgroup
    {
#pragma omp parallel num_threads(threads)
      {
#pragma omp single
#pragma omp taskloop nogroup
        for (int j = 0; j < tasks; j++) {
          usleep(50);
        }
#pragma omp taskwait
      }
    }
  }
}
