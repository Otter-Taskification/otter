module otter_serial

    contains

    subroutine fortran_otterTraceInitialise_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTraceInitialise_i(filename, functionname, linenum) bind(C, NAME="otterTraceInitialise_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterTraceInitialise_i
        end interface
        call otterTraceInitialise_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterTraceInitialise_i
    
    subroutine fortran_otterTraceFinalise()
        interface
            subroutine otterTraceFinalise() bind(C, NAME="otterTraceFinalise")
            end subroutine
        end interface
        call otterTraceFinalise()
    end subroutine fortran_otterTraceFinalise
    
    subroutine fortran_otterParallelBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterParallelBegin_i(filename, functionname, linenum) bind(C, NAME="otterParallelBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterParallelBegin_i
        end interface
        call otterParallelBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterParallelBegin_i
    
    subroutine fortran_otterParallelEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterParallelEnd() bind(C, NAME="otterParallelEnd")
            end subroutine
        end interface
    
        call otterParallelEnd()
    
    end subroutine
    
    subroutine fortran_otterTaskBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTaskBegin_i(filename, functionname, linenum) bind(C, NAME="otterTaskBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterTaskBegin_i
        end interface
    
        call otterTaskBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterTaskBegin_i
    
    subroutine fortran_otterTaskEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTaskEnd() bind(C, NAME="otterTaskEnd")
            end subroutine otterTaskEnd
        end interface
    
        call otterTaskEnd()
    end subroutine fortran_otterTaskEnd
    
    
    subroutine fortran_otterTaskSingleBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTaskSingleBegin_i(filename, functionname, linenum) bind(C, NAME="otterTaskSingleBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterTaskSingleBegin_i
        end interface
    
        call otterTaskSingleBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterTaskSingleBegin_i
    
    subroutine fortran_otterTaskSingleEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTaskSingleEnd() bind(C, NAME="otterTaskSingleEnd")
            end subroutine
        end interface
    
        call otterTaskSingleEnd()
     end subroutine fortran_otterTaskSingleEnd
    
    subroutine fortran_otterLoopBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterLoopBegin_i(filename, functionname, linenum) bind(C, NAME="otterLoopBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterLoopBegin_i
        end interface
        call otterLoopBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterLoopBegin_i
    
    subroutine fortran_otterLoopEnd()
        use, intrinsic :: iso_c_binding
        
        interface
            subroutine otterLoopEnd() bind(C, NAME="otterLoopEnd")
                use, intrinsic :: iso_c_binding
            end subroutine otterLoopEnd
        end interface
    
        call otterLoopEnd()
    end subroutine
    
    subroutine fortran_otterLoopIterationBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterLoopIterationBegin_i(filename, functionname, linenum) bind(C, NAME="otterLoopIterationBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterLoopIterationBegin_i
        end interface
        call otterLoopIterationBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterLoopIterationBegin_i
    
    subroutine fortran_otterLoopIterationEnd()
        use, intrinsic :: iso_c_binding
        
        interface
            subroutine otterLoopIterationEnd() bind(C, NAME="otterLoopIterationEnd")
                use, intrinsic :: iso_c_binding
            end subroutine otterLoopIterationEnd
        end interface
    
        call otterLoopIterationEnd()
    end subroutine
    
    subroutine fortran_otterSynchroniseChildTasks_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterSynchroniseChildTasks_i(filename, functionname, linenum) bind(C, NAME="otterSynchroniseChildTasks_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterSynchroniseChildTasks_i
        end interface
        call otterSynchroniseChildTasks_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    
    end subroutine fortran_otterSynchroniseChildTasks_i
    
    subroutine fortran_otterSynchroniseDescendantTasksBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterSynchroniseDescendantTasksBegin_i(filename, functionname, linenum)&
                    bind(C, NAME="otterSynchroniseDescendantTasksBegin_i")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterSynchroniseDescendantTasksBegin_i
        end interface
        call otterSynchroniseDescendantTasksBegin_i(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterSynchroniseDescendantTasksBegin_i
    
    subroutine fortran_otterSynchroniseDescendantTasksEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterSynchroniseDescendantTasksEnd() bind(C, NAME="otterSynchroniseDescendantTasksEnd")
            end subroutine
        end interface
        call otterSynchroniseDescendantTasksEnd()
    end subroutine fortran_otterSynchroniseDescendantTasksEnd
    
    subroutine fortran_otterTraceStart()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceStart() bind(C, NAME="otterTraceStart")
            end subroutine otterTraceStart
        end interface
        call otterTraceStart()
    end subroutine fortran_otterTraceStart
    
    subroutine fortran_otterTraceStop()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceStop() bind(C, NAME="otterTraceStop")
            end subroutine otterTraceStop
        end interface
        call otterTraceStop()
    end subroutine fortran_otterTraceStop

end module otter_serial
