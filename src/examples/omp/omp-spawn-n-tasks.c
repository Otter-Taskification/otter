#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s nTasks\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int nTasks = atoi(argv[1]);

#pragma omp parallel
#pragma omp single nowait
#pragma omp taskloop nogroup grainsize(1)
  for (int i = 0; i < nTasks; i++) {
    usleep(5);
  }

  return EXIT_SUCCESS;
}