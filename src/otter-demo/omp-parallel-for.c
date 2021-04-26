#include <omp.h>
#include <stdio.h>

#define THREADS 4
#define LEN 2

int main(void)
{
    int num[LEN*THREADS] = {0};
    int k=0, tid=0;

    printf("PARALLEL TASKLOOP\n");
    #pragma omp parallel num_threads(2)
    #pragma omp single
    #pragma omp taskloop
    for (k=0; k<LEN*THREADS; k++)
    {
        num[k] = omp_get_thread_num();
    }

    for (k=0; k<LEN*THREADS; k++)
        printf("num[%d] = %d\n", k, num[k]);


    return 0;
}