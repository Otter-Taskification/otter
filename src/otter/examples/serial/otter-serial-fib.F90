#include "otter-serial.F90"

Integer Recursive Function fib(n) result(a)
    use otter_serial
    Implicit None
    Integer, Intent(in) :: n
    Integer :: i, j

    if (n < 2) then
        a = n
    else
        ! Tag: wrap a task
        Call fortran_otterTaskBegin()
        i = fib(n-1)
        Call fortran_otterTaskEnd()

        ! Tag: wrap a task
        Call fortran_otterTaskBegin()
        j = fib(n-2)
        Call fortran_otterTaskEnd()

        ! Indicate a synchronisation constraint on a subset of work items
        Call fortran_otterSynchroniseChildTasks()

        a = i + j
    end if
End Function fib


Program fibonacci
    use otter_serial
    Implicit None
    Integer :: n, argc
    Integer :: fibn
    Integer, External :: fib
    Character(len=32) :: arg

    argc = command_argument_count()
    if (argc /= 1) then
        print *, "Provide one integer argument to the program (n)"
        STOP
    end if

    call get_command_argument(1, value=arg)
    read(arg(1:len(arg)),*) n

    Call fortran_otterTraceInitialise()

    ! Tag: start of a region we want to parallelise
    Call fortran_otterParallelBegin()

    ! Tag: wrap a task
    Call fortran_otterTaskSingleBegin()
    fibn = fib(n)
    Call fortran_otterTaskSingleEnd()

    Call fortran_otterParallelEnd()

    print *, n, fibn

    Call fortran_otterTraceFinalise()

End Program fibonacci
