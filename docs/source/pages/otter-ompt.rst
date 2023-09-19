Otter OMPT
==========

**Otter OMPT** is an OMPT plugin for tracing & visualising the structure
of loop- & task-based OpenMP programs, allowing HPC developers to see
the structure of their OpenMP 5.x programs from the perspective of the
OpenMP runtime, without any modification of the target application.
Otter uses the OpenMP Tools interface in `OpenMP
5.0 <https://www.openmp.org/spec-html/5.0/openmpch4.html>`__ to observe
OpenMP runtime events, extracting from this data the structure of the
target application independent of the particular scheduling of tasks at
runtime.

Features
--------

-  Trace the task creation and synchronisation constructs of an OpenMP
   5.x program without any modification of the source - no need to add
   any instrumentation to the target application.
-  Supports synchronisation due to taskwait and taskgroup constructs.
-  Supports nested tasks and nested parallelism.
-  No additional thread synchronisation - won't accidentally serialise
   the target application.

Example
-------

Take this example code which uses nested tasks synchronised by a
taskwait barrier to calculate the nth Fibonacci number:

.. code:: c

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

We can speculate about the structure of this code in terms of tasks and
their synchronisation, but how can we check that our ideas match
reality? This is a challenge even for the simple code above, and soon
becomes impossible for complex task-based code. We might try using
performance analysis tools to trace or profile an application, providing
a thread-centric view of a specific arrangement of tasks. While this
gives us insight into the application's runtime performance, we would
still struggle to get a clear picture of the application's overall
structure. Using Otter we can observe the true structure of a task-based
OpenMP application, all without modifying the application's source.

Getting Started
---------------

See the :doc:`installation </source/pages/installation>` page for building &
installing Otter.

Tracing a target OpenMP application ``omp-demo`` is as simple as:

.. code:: bash

   OMP_TOOL_LIBRARIES=<otter-install-prefix>/lib/libotter-ompt.so ./omp-demo

If everything is set up correctly, you should see output like this
(depending on the specific OpenMP runtime you are using):

::

   Intel(R) OMP version: 5.0.20210428, OMP v. 201611
   Otter was compiled with icc (ICC) 2021.3.0 20210609
   Starting OTTer...
   Trace output path:             trace/otter_trace.[pid]

   Registering callbacks:
   ompt_callback_thread_begin       | ompt_set_always (5)
   ompt_callback_thread_end         | ompt_set_always (5)
   ompt_callback_parallel_begin     | ompt_set_always (5)
   ompt_callback_parallel_end       | ompt_set_always (5)
   ompt_callback_task_create        | ompt_set_always (5)
   ompt_callback_task_schedule      | ompt_set_always (5)
   ompt_callback_implicit_task      | ompt_set_always (5)
   ompt_callback_work               | ompt_set_always (5)
   ompt_callback_masked             | ompt_set_always (5)
   ompt_callback_sync_region        | ompt_set_always (5)

   PROCESS RESOURCE USAGE:
          maximum resident set size:    11916 kb
   page reclaims (soft page faults):     5644 
     page faults (hard page faults):        1 
             block input operations:        0 
            block output operations:       72 

                            threads:        3 
                   parallel regions:        2 
                              tasks:       25 
   OTTER_TRACE_FOLDER=/home/adam/otter/trace/otter_trace.[pid]

By default, Otter writes a trace to ``trace/otter_trace.[pid]`` - the
location and name of the trace can be set with the ``OTTER_TRACE_PATH``
and ``OTTER_TRACE_NAME`` environment variables.
