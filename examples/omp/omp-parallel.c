#include <omp.h>

#define THREADS 2

int main(void) {
#pragma omp parallel num_threads(THREADS)
  {
    // pass
  }
}