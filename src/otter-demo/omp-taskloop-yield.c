#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2
#define LEN 25
#define USLEEP 3
#define SLEEP 30

void long_task(void)
{
    int k = 0;
    for (k=0; k<1000; k++)
    {
        #pragma omp taskyield
        usleep(USLEEP);
    }
}

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    #pragma omp single
    #pragma omp taskloop grainsize(1)
    for (j=0; j<LEN; j++)
    {
        if (j<THREADS)
        {
            long_task();
        } else {
            usleep(SLEEP);
        }

    }

    return 0;
}
