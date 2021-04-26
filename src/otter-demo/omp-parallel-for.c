#include <omp.h>
#include <stdio.h>

#define THREADS 4
#define LEN 2

int main(void)
{
    int num[LEN*THREADS] = {0};
    int k=0, tid=0;

    printf("PARALLEL FOR\n");
    #pragma omp parallel num_threads(THREADS)
    {
        tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (k=0; k<LEN*THREADS; k++)
        {
            num[k] = tid;
        }
    }

    for (k=0; k<LEN*THREADS; k++)
        printf("num[%d] = %d\n", k, num[k]);

    printf("PARALLEL TASKLOOP\n");
    #pragma omp parallel num_threads(THREADS)
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