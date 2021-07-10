#include <stdio.h>
#include <omp.h>

#define THREADS 4

int main(void)
{
    #pragma omp parallel num_threads(THREADS) if(0)
    {
        #pragma omp single
        printf("There are %d threads\n", omp_get_num_threads());
    }
}
