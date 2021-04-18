#include <omp.h>

int main(void)
{
    #pragma omp parallel num_threads(1)
    {
        #pragma omp single
        #pragma omp task
        {
            #pragma omp taskloop
            for (int i=0; i<3; i++)
            {
            }
        }
    }
    return 0;
}