#include <omp.h>
#include <stdio.h>

#define LEN 20

int main(void) {
  int num[LEN] = {0}, k = 0;
#pragma omp parallel for
  for (k = 0; k < LEN; k++) {
    num[k] = omp_get_thread_num();
  }

  return 0;
}
