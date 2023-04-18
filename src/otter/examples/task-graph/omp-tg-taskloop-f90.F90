#include "otter/otter-task-graph.F90"

Program taskloop
    use, intrinsic :: iso_c_binding
    use otter_task_graph
    Implicit None
    Integer :: i
    Type(c_ptr) :: tloop
    Type(c_ptr) :: root
    Call fortran_otterTraceInitialise()
    !$omp parallel private(i)
    !$omp single
    tloop = fortran_otterTaskBegin(c_null_ptr)
    !$omp taskloop grainsize(32) private(root) nogroup
    Do i=1, 128
        if (MOD(i, 32) == 1) then
            root = fortran_otterTaskBegin(tloop)
        end if
        call sleep(1) ! sleeps for 1 sec
        if (MOD(i, 32) == 0) then
            Call fortran_otterTaskEnd(root)
        end if
    End Do
    !$omp end taskloop
    Call fortran_otterSynchroniseTasks(tloop, otter_sync_children)
    Call fortran_otterTaskEnd(tloop)
    !$omp end single
    !$omp end parallel

    Call fortran_otterTraceFinalise()
end Program taskloop
