#include "otter/otter-task-graph.F90"

Integer Recursive Function fib(n, parent) result(a)
    use, intrinsic :: iso_c_binding
    use otter_task_graph
    Implicit None
    Integer, Intent(In) :: n
    type(c_ptr) :: parent
    type(c_ptr) :: child1, child2
    Integer i, j
    if (n < 2) then
        a = n
    else
        ! Tag: wrap a task
        child1 = fortran_otterTaskBegin(parent)
        !$omp task shared(i, child1, n)
        i = fib(n-1, child1)
        Call fortran_otterTaskEnd(child1)
        !$omp end task

        ! Tag: wrap a task
        child2 = fortran_otterTaskBegin(parent)
        !$omp task shared(i, child2, n)
        j = fib(n-2, child2)
        Call fortran_otterTaskEnd(child2)
        !$omp end task

        ! Indicate a synchronisation constraint on a subset of work items
        ! 0 indicates synchronise children only (not all descendants)
        !$omp taskwait
        Call fortran_otterSynchroniseTasks(parent, otter_sync_children)

        a = i + j
    end if
End Function fib

Program fibonacci
    use, intrinsic :: iso_c_binding
    use otter_task_graph
    Implicit None
    Integer :: n, argc
    Integer :: fibn
    Integer, External :: fib
    Character(len=32) :: arg
    Type(c_ptr) :: root

    argc = command_argument_count()
    if (argc /= 1) then
        print *, "Provide one integer argument to the program (n)"
        STOP
    end if

    call get_command_argument(1, value=arg)
    read(arg(1:len(arg)),*) n

    Call fortran_otterTraceInitialise()
    !$omp parallel shared(fibn, n)
    !$omp single
    ! Tag: wrap a task
    root = fortran_otterTaskBegin(c_null_ptr)
    fibn = fib(n, root)
    Call fortran_otterTaskEnd(root)
    !$omp end single
    !$omp end parallel

    print *, n, fibn

    Call fortran_otterTraceFinalise()

End Program fibonacci
