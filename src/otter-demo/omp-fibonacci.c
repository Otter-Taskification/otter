#include <stdio.h>
#include <omp.h>

int f(int n) {
    int i, j;
    if (n<2) return n;

    #pragma omp task shared(i) firstprivate(n)
        i = f(n-1);

    #pragma omp task shared(j) firstprivate(n)
        j = f(n-2);

    #pragma omp taskwait

    return i+j;
}

int main(int argc, char *argv[]) {

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    #pragma omp parallel shared(n) num_threads(4)
    {
        #pragma omp single
        printf("f(%d) = %d\n", n, f(n));
    }
}