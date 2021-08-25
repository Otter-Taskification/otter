# Otter - An OMPT Tool for Tracing OpenMP Tasks

Otter is a tool for visualising the structure of task-based OpenMP programs allowing developers and researchers to see the true structure of their OpenMP programs from the perspective of the OpenMP runtime, without any modification of the target application.

Otter uses the OpenMP Tools interface in [OpenMP 5.0](https://www.openmp.org/spec-html/5.0/openmpch4.html) to observe task creation and synchronisation events, extracting from this data the structure of a target application independent of the particular scheduling of tasks at runtime.

Take this example code which uses nested tasks synchronised by a taskwait barrier to calculate the n<sup>th</sup> Fibonacci number:

```c
int fibonacci(int n) {
    int i, j;
    if (n<2) return n;
    #pragma omp task shared(i) firstprivate(n)
        i = f(n-1);
    #pragma omp task shared(j) firstprivate(n)
        j = f(n-2);
    #pragma omp taskwait
    return i+j;
}
```

We can speculate about the structure of this code in terms of tasks and their synchronisation, but how can we check that our ideas match reality? This is a challenge even for the simple code above, and soon becomes impossible for complex task-based code. We might try using performance analysis tools to trace or profile an application, providing a thread-centric view of a specific arrangement of tasks. While this gives us insight into the application's runtime performance, we would still struggle to get a clear picture of the application's overall structure. Using Otter we can observe the true structure of a task-based OpenMP application, all without modifying the application's source. Here is the result of applying Otter to a program using the Fibonacci function above:

<p align="center">
<img src="docs/listing2.svg" height="750" alt="The task-based structure of the Fibonacci function.">
</p>

The nodes of this graph represent the different OpenMP constructs that Otter can show:

<p align="center">
<img src="docs/node-symbol-table.svg" height="200" alt="The node styles representing the OpenMP constructs represented by Otter.">
</p>
