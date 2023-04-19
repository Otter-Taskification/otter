#include "api/otter-task-graph/otter-task-graph.F90"

Program taskloop
    use, intrinsic :: iso_c_binding
    use otter_task_graph
    Implicit None
    Integer :: i
    Type(c_ptr) :: task1, task2
    Type(c_ptr) :: root
    Call fortran_otterTraceInitialise()

    task1 = fortran_otterTaskBegin(c_null_ptr)
    Call fortran_otterSynchroniseTasks(task1, otter_sync_children)
    Call fortran_otterTaskEnd(task1)

    task2 = fortran_otterTaskBegin(c_null_ptr)
    call fortran_otterSynchroniseTasks(task2, otter_sync_descendants)
    call fortran_otterTaskEnd(task2)

    Call fortran_otterTraceFinalise()
end Program taskloop
