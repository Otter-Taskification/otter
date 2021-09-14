#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    if(argc == 1 || argc >= 4)
    {
        fprintf(stderr, "Usage: %s nTasks [yield (default:1)]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nTasks = atoi(argv[1]);
    bool yield = true;
    if (argc == 3)
        yield = (atoi(argv[2]) ? true : false);

    #pragma omp parallel
    #pragma omp single nowait
    #pragma omp task
    #pragma omp taskloop nogroup grainsize(1)
    for (int i=0; i<nTasks; i++){
        usleep(1000);
        if (yield) {
            #pragma omp taskyield
        }
    }

    return EXIT_SUCCESS;
}