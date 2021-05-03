#include <omp.h>
#include <stdio.h>

#define THREADS 1
#define LEN 2

int main(void)
{
    int num[LEN*THREADS] = {0};
    int k=0, tid=0;

    omp_set_num_threads(THREADS);

    printf("PARALLEL TASKLOOP\n");
    #pragma omp parallel
    #pragma omp for
    for (k=0; k<3; k++)
    {
        #pragma omp task
        num[k] = omp_get_thread_num();
    }

    #pragma omp parallel for
    for (k=0; k<5; k++)
    {
        #pragma omp task
        num[k] = omp_get_thread_num();
    }
    // #pragma omp taskwait

    printf("PARALLEL TASKLOOP\n");
    #pragma omp parallel
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
