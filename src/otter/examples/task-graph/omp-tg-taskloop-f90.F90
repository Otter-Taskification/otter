#include "otter/otter-task-graph.F90"

Program taskloop
    use, intrinsic :: iso_c_binding
    use otter_task_graph
    Implicit None
    Integer :: i
    Type(c_ptr) :: root
    Call fortran_otterTraceInitialise()
    !$omp parallel private(i)
    !$omp single
    !$omp taskloop grainsize(32) private(root)
    Do i=1, 128
        if (MOD(i, 32) == 1) then
            root = fortran_otterTaskBegin(c_null_ptr)
        end if
        call sleep(1) ! sleeps for 1 sec
        if (MOD(i, 32) == 0) then
            Call fortran_otterTaskEnd(root)
        end if
    End Do
    !$omp end taskloop
    !$omp end single
    !$omp end parallel

    Call fortran_otterTraceFinalise()
end Program taskloop
