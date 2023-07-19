#define _GNU_SOURCE

#include <omp.h>
#include <sched.h>
#include <stdio.h>

#define THREADS 8

int main(void) {
  int i = 0;
#pragma omp parallel num_threads(THREADS)
  {
    printf("Thread %d/%d on cpu %d\n", omp_get_thread_num(),
           omp_get_num_threads(), sched_getcpu());
  }
  return 0;
}
