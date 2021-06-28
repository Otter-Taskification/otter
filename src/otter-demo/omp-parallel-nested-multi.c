#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2

int main(void)
{
    omp_set_max_active_levels(4);

    #pragma omp parallel num_threads(THREADS)
    {
        int j = omp_get_thread_num();
        #pragma omp parallel num_threads(3)
        {
            #pragma omp parallel num_threads(2)
            {
            printf("%d: %d/%d level=%d\n",
                j, omp_get_thread_num(), omp_get_num_threads(), omp_get_level());
            }
            
            #pragma omp parallel num_threads(2)
            {
            printf("%d: %d/%d level=%d\n",
                j, omp_get_thread_num(), omp_get_num_threads(), omp_get_level());
            }
        }
    }

}
