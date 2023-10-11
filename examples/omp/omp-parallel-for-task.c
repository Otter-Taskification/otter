#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define THREADS 2
#define LEN 7

int main(void) {
  int num[LEN] = {0}, k = 0;
  omp_set_num_threads(THREADS);
#pragma omp parallel for
  for (k = 0; k < LEN; k++) {
#pragma omp task
    {
      num[k] = omp_get_thread_num();
      usleep(50);
    }
  }

  for (k = 0; k < LEN; k++) {
    printf("%d\n", num[k]);
  }

  return 0;
}
