#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 6
#define LEN 2

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    {
        // #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                usleep(30);
            }
        }
        #pragma omp taskwait
    }

    return 0;
}
