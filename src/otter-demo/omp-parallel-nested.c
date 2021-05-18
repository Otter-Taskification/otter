#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 1

int main(void)
{
    omp_set_max_active_levels(2);

    #pragma omp parallel num_threads(THREADS)
    {
        int j = omp_get_thread_num();
        #pragma omp parallel num_threads(THREADS)
        {
            printf("%d: %d/%d level=%d\n",
                j, omp_get_thread_num(), omp_get_num_threads(), omp_get_level());
        }
    }

}
