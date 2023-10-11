#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s num_threads num_tasks\n", argv[0]);
    return 1;
  }

  int threads = atoi(argv[1]);
  int tasks = atoi(argv[2]);

#pragma omp parallel num_threads(threads)
  {
#pragma omp master
    {
      int j = 0;
      printf("[%d] I am the master thread!\n", omp_get_thread_num());
#pragma omp taskloop nogroup
      for (j = 0; j < tasks; j++) {
        printf("[%d] loop %d\n", omp_get_thread_num(), j);
      }
    }

#pragma omp barrier

#pragma omp master
    {
      int j = 0;
      printf("[%d] I am the master thread!\n", omp_get_thread_num());
#pragma omp taskloop nogroup
      for (j = 0; j < tasks; j++) {
        printf("[%d] loop %d\n", omp_get_thread_num(), j);
      }
    }
  }
}
