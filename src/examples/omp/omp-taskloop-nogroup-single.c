#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define LEN 4

int main(void)
{
    int j=0;
    #pragma omp parallel
    {
        #pragma omp single
        {
        #pragma omp taskloop nogroup
        for (j=0; j<LEN; j++)
        {
            usleep(30);
        }
        #pragma omp taskwait
        }
    }

    return 0;
}
