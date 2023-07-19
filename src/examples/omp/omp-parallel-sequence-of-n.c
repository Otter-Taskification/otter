#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s num_threads num_regions\n", argv[0]);
    return 1;
  }

  int threads = atoi(argv[1]);
  int regions = atoi(argv[2]);

  for (int k = 0; k < regions; k++) {
#pragma omp parallel num_threads(threads)
    {
      // pass
    }
  }
}
