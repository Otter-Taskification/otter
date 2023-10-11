#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define THREADS 2
#define LEN 2

int main(void) {
  int counter[THREADS] = {0};
#pragma omp parallel num_threads(THREADS)
  {
#pragma omp taskgroup
    for (counter[omp_get_thread_num()] = 0; counter[omp_get_thread_num()] < LEN;
         counter[omp_get_thread_num()]++) {
#pragma omp task
      {
#pragma omp task
          {usleep(10);
    }
#pragma omp task
    { usleep(10); }
#pragma omp task
    { usleep(10); }
  }

// Add an empty task so that the taskgroup-end vertex will synchronise
// a vertex with both the taskswitch-enter & -leave events (test assertions)
#pragma omp task
  { usleep(10); }
}

#pragma omp taskgroup
for (counter[omp_get_thread_num()] = 0; counter[omp_get_thread_num()] < LEN;
     counter[omp_get_thread_num()]++) {
#pragma omp task
  {
#pragma omp task
    {
#pragma omp task
        {usleep(10);
  }
#pragma omp task
  { usleep(10); }
}
#pragma omp task
{ usleep(10); }
}
}
}

return 0;
}
