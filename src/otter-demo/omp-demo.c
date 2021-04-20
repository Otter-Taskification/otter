#include <omp.h>

int main(void)
{
    int i=0;
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        #pragma omp task
        {
            #pragma omp taskloop
            for (i=0; i<5; i++)
            {
            }
        }
    }
    return 0;
}