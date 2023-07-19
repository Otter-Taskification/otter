#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  int j = 0;
#pragma omp parallel
  {
#pragma omp single
    {
#pragma omp task
      { usleep(10); }
    }
#pragma omp taskwait
  }

  return 0;
}
