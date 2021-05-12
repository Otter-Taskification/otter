#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 4
#define LEN 25

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    #pragma omp single
    #pragma omp taskgroup
    for (j=0; j<LEN; j++)
    {
        #pragma omp task
        {usleep(10);}
    }

    return 0;
}
