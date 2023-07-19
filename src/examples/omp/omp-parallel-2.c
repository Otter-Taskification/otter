#include <omp.h>

#define THREADS 2

int main(void) {
#pragma omp parallel num_threads(THREADS)
  {
      // pass
  }

#pragma omp parallel num_threads(THREADS * 4)
  {
      // pass
  }

#pragma omp parallel num_threads(THREADS * 2)
  {
    // pass
  }
}