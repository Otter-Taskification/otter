#include <omp.h>
#include <stdio.h>

#define THREADS     4
#define LOOPS       6

int main(void) {

    #pragma omp parallel num_threads(4)
    {
        #pragma omp master
        {
            int j=0;
            printf("[%d] I am the master thread!\n", omp_get_thread_num());
            #pragma omp taskloop nogroup
            for (j=0; j<LOOPS; j++)
            {
                printf("[%d] loop %d\n", omp_get_thread_num(), j);
            }
        }
    }
}