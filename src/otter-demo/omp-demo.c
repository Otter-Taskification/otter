#include <omp.h>

#define LOOPS 2

int main(void)
{
    int i=0;
    #pragma omp parallel num_threads(3)
    {
        #pragma omp task
        {
            #pragma omp taskloop
            for (i=0; i<LOOPS; i++)
            {
            }
        }
        #pragma omp task
        {}
    }
    #pragma omp parallel num_threads(3)
    {
        #pragma omp task
        {
            #pragma omp taskloop
            for (i=0; i<LOOPS; i++)
            {
            }
        }
        #pragma omp task
        {}
    }
    return 0;
}
