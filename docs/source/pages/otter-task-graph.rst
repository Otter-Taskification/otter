Otter task-graph
================

As part of the `ExCALIBUR cross-cutting task parallelism research
theme <https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/>`__,
the **Otter task-graph** API is designed to facilitate data-driven
parallelisation of multi-threaded code. Using the API, developers
annotate regions of code which conceptually may represent tasks, along
with any synchronisation constraints between tasks. By re-compiling and
linking to the Otter task-graph library, the task-based structure of the
annotated code can be traced. The recorded trace data is used by PyOtter
to visualise the annotated code as a task graph and to report the
recommended parallelisation strategy.

`This video <https://www.youtube.com/watch?v=XR6mRvD7-Cg>`__ explains
the vision behind the Otter API.

The Otter task graph API
------------------------

The user-facing API, which consists of a set of function-like macros, is
provided by the ``otter/otter-task-graph-user.h`` header. See the
documentation in that header for the canonical usage instructions. These
instructions are summarised below.

Before the API can be used it must be initialised with
``OTTER_INITIALISE()`` and when tracing is complete it must be finalised
with ``OTTER_FINALISE()``.

The sections below highlight the macros used when annotating application
code. For a full explanation, refer to the documentation in the header
file.

Labelling and storing task handles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Otter uses opaque handles to refer to instances of annotated tasks and
the dependencies between them. To allow task handles to be used across
scopes, they can be stored internally in user-labelled task pools. The
task-pool labels may be parameterised with user-supplied data using
``printf``-like format strings as the labels. The task pools themselves
are created upon demand. This allows the user to define, store and refer
to distinct tasks across separate scopes.

.. important ::

    Tasks in the same task pool (i.e. with the same label) cannot
    be distinguished and must be considered logically interchangeable.

Declaring and defining tasks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These macros do not record any trace events.

+--------------------------------------------------------------+--------------------------------------------------+
| Macro                                                        | Usage                                            |
+==============================================================+==================================================+
| ``OTTER_DECLARE_HANDLE(task)``                               | Declare a task handle ``task`` in the current    |
|                                                              | scope.                                           |
+--------------------------------------------------------------+--------------------------------------------------+
| ``OTTER_INIT_TASK(task, parent, add_to_pool, label, ...)``   | Initialise the handle ``task`` with a new task   |
|                                                              | instance. If ``add_to_pool`` is                  |
|                                                              | ``otter_add_to_pool``, store the task in the     |
|                                                              | task pool with the given label.                  |
+--------------------------------------------------------------+--------------------------------------------------+
| ``OTTER_DEFINE_TASK(task, parent, add_to_pool, label, ...)`` | Declare and initialise a new task handle in the  |
|                                                              | current scope (equivalent to                     |
|                                                              | ``OTTER_DECLARE_HANDLE()`` followed by           |
|                                                              | ``OTTER_INIT_TASK()``).                          |
+--------------------------------------------------------------+--------------------------------------------------+

Storing and retrieving tasks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These macros do not record any trace events.

+-----------------------------------------------+-----------------------------------------------------+
| Macro                                         | Usage                                               |
+===============================================+=====================================================+
| ``OTTER_POOL_ADD(task, label, ...)``          | Add a task handle to the task pool associated with  |
|                                               | the given label.                                    |
|                                               |                                                     |
+-----------------------------------------------+-----------------------------------------------------+
| ``OTTER_POOL_POP(task, label, ...)``          | Assign a task removed from the task pool with the   |
|                                               | given label (or ``OTTER_NULL_TASK`` if none         |
|                                               | available).                                         |
+-----------------------------------------------+-----------------------------------------------------+
|                                               | Declare a handle and assign a task removed from the |
| ``OTTER_POOL_DECL_POP(task, label, ...)``     | task pool with the given label (or                  |
|                                               | ``OTTER_NULL_TASK`` if none available).             |
|                                               |                                                     |
+-----------------------------------------------+-----------------------------------------------------+
| ``OTTER_POOL_BORROW(task, label, ...)``       | Assign a task borrowed from the task pool           |
|                                               | associated with the given label (or                 |
|                                               | ``OTTER_NULL_TASK`` if none available).             |
+-----------------------------------------------+-----------------------------------------------------+
|                                               | Declare a handle and borrow a task from the task    |
| ``OTTER_POOL_DECL_BORROW(task, label, ...)``  | pool associated with the given label (or            |
|                                               | ``OTTER_NULL_TASK`` if none available).             |
|                                               |                                                     |
+-----------------------------------------------+-----------------------------------------------------+

Annotating task start, end and synchronisation constraints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These macros record trace events.

+--------------------------------------------+-----------------------------------------------------+
| Macro                                      | Usage                                               |
+============================================+=====================================================+
| ``OTTER_TASK_START(task)``                 | Record the start of the code represented by the     |
|                                            | given task handle.                                  |
+--------------------------------------------+-----------------------------------------------------+
| ``OTTER_TASK_END(task)``                   | Record the end of the code represented by the given |
|                                            | task handle.                                        |
+--------------------------------------------+-----------------------------------------------------+
| ``OTTER_TASK_WAIT_FOR(task, mode)``        | Records a barrier where the given task must wait    |
|                                            | until all prior child or descendant tasks are       |
|                                            | complete.                                           |
+--------------------------------------------+-----------------------------------------------------+

Managing global phases
~~~~~~~~~~~~~~~~~~~~~~

These macros record trace events.

+--------------------------------+----------------------------------------------------+
| Macro                          | Usage                                              |
+================================+====================================================+
| ``OTTER_PHASE_BEGIN(name)``    | Start a new global algorithmic phase with the      |
|                                | given name.                                        |
+--------------------------------+----------------------------------------------------+
| ``OTTER_PHASE_END()``          | End the present global algorithmic phase.          |
|                                |                                                    |
+--------------------------------+----------------------------------------------------+
| ``OTTER_PHASE_SWITCH(name)``   | End the present global algorithmic phase and       |
|                                | immediately switch to another.                     |
+--------------------------------+----------------------------------------------------+

A stub version of ``otter/otter-task-graph-user.h``, which provides a
stub version of the API, is installed alongside Otter-Task-Graph and is
also available
`here <https://github.com/Otter-Taskification/otter/blob/dev/include/api/otter-task-graph/otter-task-graph-stub.h>`__
for use by users of software which itself includes or uses Otter but
does not require it's own users to install Otter. This file is placed
into the public domain (see the license in that file).

Annotating with Otter
---------------------

This page explains how to use Otter to annotate and trace the structure
of a simple example program which calculates the nth Fibonacci number.

The program to be annotated is:

.. code:: c

   #include <stdio.h>
   #include <stdlib.h>

   int fib(int n);

   int main(int argc, char *argv[]) {

       if (argc != 2) {
           fprintf(stderr, "usage: %s n\n", argv[0]);
           return 1;
       }

       int n = atoi(argv[1]);
       int fibn = 0;

       // The main calculation which we'd like to annotate as a task
       fibn = fib(n);

       printf("f(%d) = %d\n", n, fibn);

       return 0;
   }

   int fib(int n) {
       if (n<2) return n;
       int i, j;

       // Each call to fib() spawns 2 further calls
       i = fib(n-1);
       j = fib(n-2);

       // Output dependency on i and j
       return i+j;
   }

1. Annotate the target application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Before the API can be used it must be initialised with
``OTTER_INITIALISE()`` and it must be finalised with
``OTTER_FINALISE()`` immediately before the program exits. All call to
the API must occur between these initialisation & finalisation calls.
The API can therefore be initialised in this way:

.. code:: c

   #include <otter/otter-task-graph.h>

   int main(int argc, char *argv[]) {

       OTTER_INITIALISE();

       // Main body of program
       {
           fibn = fib(n);
       }

       OTTER_FINALISE();

       return 0;
   }

Each section of code representing a potential task should be annotated
with calls to ``OTTER_TASK_[START|END]()`` e.g.

.. code:: c

   OTTER_INITIALISE();
   {
       OTTER_DEFINE_TASK(root, OTTER_NULL_TASK, otter_add_to_pool, "fib(%d)", n);
       OTTER_TASK_START(root);
       fibn = fib(n);
       OTTER_TASK_END(root);
   }
   OTTER_FINALISE();

In this example, each recursive call to ``fib()`` can be considered as a
task. In order to record parent-child links between these calls, it is
necessary to refer to the handle of an enclosing task. This is done with
the ``OTTER_POOL_DECL_POP()`` macro. Because there is an output
dependency on ``i`` and ``j``, each task representing a call to
``fib()`` must record a barrier for the result of the tasks it spawns.
This constraint is specified with
``OTTER_TASK_WAIT_FOR(parent, children)``:

.. code:: c

   int fib(int n) {
       if (n<2) return n;
       int i, j;

       // refer to the parent task
       OTTER_POOL_DECL_POP(parent, "fib(%d)", n);

       // indicate a task
       OTTER_DEFINE_TASK(child1, parent, otter_add_to_pool, "fib(%d)", n - 1);
       OTTER_TASK_START(child1);
       i = fib(n - 1);
       OTTER_TASK_END(child1);

       // indicate a task
       OTTER_DEFINE_TASK(child2, parent, otter_add_to_pool, "fib(%d)", n - 2);
       OTTER_TASK_START(child2);
       j = fib(n - 2);
       OTTER_TASK_END(child2);

       // Indicate a synchronisation constraint
       OTTER_TASK_WAIT_FOR(parent, children);

       return i+j;
   }

2. Compile the annotated target
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The annoted program ``fib.c`` can be compiled with:

::

   clang fib.c -lotter-task-graph -lotf2 -o fib

Use ``-L`` to specify the installation directories for OTF2 and
Otter-Task-Graph if these were not installed to a standard location.

3. Obtain a trace
~~~~~~~~~~~~~~~~~

Running the annotated executable will cause a trace to be generated. The
location of the trace can be controlled using the ``OTTER_TRACE_PATH``
and ``OTTER_TRACE_NAME`` environment variables. By default, trace files
are written to ``trace/``. The program will report the location of the generated
trace file:

::

   OTTER_TRACE_FOLDER=trace/otter_trace.[pid]
