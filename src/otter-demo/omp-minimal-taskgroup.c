#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2
#define LEN 2

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    #pragma omp taskgroup
    {

    }

    return 0;
}
