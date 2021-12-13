#include <omp.h>
#include <unistd.h>
#include <stdio.h>

#define THREADS 4
#define LEN 5

int main(void)
{
    int j=0;
    #pragma omp parallel num_threads(THREADS)
    {
        #pragma omp single nowait
        {
            #pragma omp taskloop nogroup
            for (j=0; j<LEN; j++)
            {
                usleep(50);
            }
        }
    }

    return 0;
}
