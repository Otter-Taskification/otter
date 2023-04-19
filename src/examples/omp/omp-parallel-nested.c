#include <omp.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s threads\n", argv[0]);
        return 1;
    }

    int threads = atoi(argv[1]);

    omp_set_max_active_levels(2);

    #pragma omp parallel num_threads(threads)
    {
        int j = omp_get_thread_num();
        #pragma omp parallel num_threads(threads)
        {
            printf("%d: %d/%d level=%d\n",
                j, omp_get_thread_num(), omp_get_num_threads(), omp_get_level());
        }
    }

}
