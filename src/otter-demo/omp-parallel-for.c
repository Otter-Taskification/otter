#include <omp.h>
#include <stdio.h>

#define THREADS 2
#define LEN 20

int main(void)
{
    int num[LEN] = {0}, k=0;
    omp_set_num_threads(THREADS);
    #pragma omp parallel for
    for (k=0; k<LEN; k++)
    {
        num[k] = omp_get_thread_num();
    }

    return 0;
}
