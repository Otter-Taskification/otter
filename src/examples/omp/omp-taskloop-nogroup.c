#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define THREADS 6
#define LEN 4

int main(void) {
  int j = 0;
#pragma omp parallel num_threads(THREADS)
  {
#pragma omp taskloop nogroup
    for (j = 0; j < LEN; j++) {
      usleep(30);
    }
#pragma omp taskwait
  }

  return 0;
}
