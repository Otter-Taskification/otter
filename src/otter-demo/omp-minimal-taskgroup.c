#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2
#define LEN 5

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    #pragma omp single
    #pragma omp taskloop nogroup
    for (j=0; j<LEN; j++)
    {
        usleep(10);
    }

    return 0;
}
