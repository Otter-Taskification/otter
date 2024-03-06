module fib_mod
    use, intrinsic :: iso_c_binding

    Logical(c_bool), Parameter :: c_true = .true.
    contains

Integer Recursive Function fib(n) result(val)
    use otter_task_graph
    use, intrinsic :: iso_c_binding
    implicit none
    Integer, intent(in) :: n
    Integer :: i, j
    Type(c_ptr) :: parent
    Type(c_ptr) :: child1
    Type(c_ptr) :: child2
    character(len=8) :: char_n

    if(n < 2) then
        val = 1
        return
    end if

    write(char_n, '(I5)') n
    !OTTER_POP_LABEL
    parent = fortran_otterTaskPopLabel("fib("//TRIM(char_n)//")")

    child1 = fortran_otterTaskInitialise(parent, -1, otter_add_to_pool, c_true, &
                                         __FILE__, "fib", __LINE__, &
                                         "fib("//TRIM(char_n)//").child1")
    i = fib(n-1)
    call fortran_otterTaskEnd(child1, __FILE__, "fib", __LINE__)

    child2 = fortran_otterTaskInitialise(parent, -1, otter_add_to_pool, c_true, &
                                         __FILE__, "fib", __LINE__,&
                                         "fib("//TRIM(char_n)//").child2")
    j = fib(n-2)
    call fortran_otterTaskEnd(child2, __FILE__, "fib", __LINE__)

    call fortran_otterSynchroniseTasks(parent, 1, otter_endpoint_discrete)

    val = i + j
End Function fib

end module

Program fibonacci
    use otter_task_graph
    use fib_mod
    use, intrinsic :: iso_c_binding
    implicit none
    CHARACTER(len=32) :: arg
    integer :: num, stat
    Type(c_ptr) :: parent = c_null_ptr
    integer :: fibn
    character(len=8) :: char_n

    CALL GET_COMMAND_ARGUMENT(1, arg)
    if(len_trim(arg) == 0) then
        print *, "Requires 1 argument"
        call ABORT()
    end if
    read(arg,*,iostat=stat) num
    !    num = Integer(arg)

    write(char_n, '(I5)') num
    call fortran_otterTraceInitialise(__FILE__, "main", __LINE__)

    call fortran_otterPhaseBegin("calculate_fib("//TRIM(char_n)//")",&
        __FILE__, "main", __LINE__)

    parent = fortran_otterTaskInitialise(parent, -1, otter_add_to_pool, c_true, &
                                         __FILE__, "fib", __LINE__, &
                                         "fib("//TRIM(char_n)//")")
    parent= fortran_otterTaskStart(parent, __FILE__, "main", __LINE__)
    fibn = fib(num)
    call fortran_otterTaskEnd(parent, __FILE__, "main", __LINE__)
    
    call fortran_otterSynchroniseTasks(c_null_ptr, 1, otter_endpoint_discrete)

    call fortran_otterPhaseEnd(__FILE__, "main", __LINE__)

    print *, "fib(", num, ") = ", fibn

    call fortran_otterTraceFinalise(__FILE__, "main", __LINE__)

End Program
