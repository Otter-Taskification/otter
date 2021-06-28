#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2
#define LEN 7

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    {
        #pragma omp taskloop
        for (j=0; j<LEN; j++)
        {
            usleep(30);
        }
    }

    return 0;
}
