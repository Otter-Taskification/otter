#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 2
#define LOOPS   3

void work(int k, int t)
{
    #pragma omp parallel num_threads(THREADS)
    {
        printf("%d/%d in region %d at level %d (encountering thread is %d)\n",
            omp_get_thread_num(),
            omp_get_num_threads(),
            k,
            omp_get_level(),
            t
        );
        usleep(30);
    }
}

int main(void)
{
    int j=0;
    omp_set_max_active_levels(2);
    #pragma omp parallel num_threads(THREADS)
    {
        #pragma omp single
        #pragma omp taskloop grainsize(1)
        for (j=0; j<LOOPS; j++)
        {
            work(j, omp_get_thread_num());
        }
    }
}
