module otter_serial

    contains

    subroutine fortran_otterTraceInitialise_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTraceInitialise(filename, functionname, linenum) bind(C, NAME="otterTraceInitialise")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterTraceInitialise
        end interface
        call otterTraceInitialise(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterTraceInitialise_i
    
    subroutine fortran_otterTraceFinalise()
        interface
            subroutine otterTraceFinalise() bind(C, NAME="otterTraceFinalise")
            end subroutine
        end interface
        call otterTraceFinalise()
    end subroutine fortran_otterTraceFinalise
    
    subroutine fortran_otterThreadsBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterThreadsBegin(filename, functionname, linenum) bind(C, NAME="otterThreadsBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterThreadsBegin
        end interface
        call otterThreadsBegin(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterThreadsBegin_i
    
    subroutine fortran_otterThreadsEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterThreadsEnd() bind(C, NAME="otterThreadsEnd")
            end subroutine
        end interface
    
        call otterThreadsEnd()
    
    end subroutine
    
    subroutine fortran_otterTaskBegin_i(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTaskBegin(filename, functionname, linenum) bind(C, NAME="otterTaskBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
            end subroutine otterTaskBegin
        end interface
    
        call otterTaskBegin(trim(filename), trim(functionname), Int(linenum, kind=c_int))
    end subroutine fortran_otterTaskBegin_i
    
    subroutine fortran_otterTaskEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTaskEnd() bind(C, NAME="otterTaskEnd")
            end subroutine otterTaskEnd
        end interface
    
        call otterTaskEnd()
    end subroutine fortran_otterTaskEnd
    
    subroutine fortran_otterLoopBegin_i()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterLoopBegin() bind(C, NAME="otterLoopBegin")
                use, intrinsic :: iso_c_binding
            end subroutine otterLoopBegin
        end interface
        call otterLoopBegin()
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
    
    subroutine fortran_otterLoopIterationBegin_i()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterLoopIterationBegin() bind(C, NAME="otterLoopIterationBegin")
                use, intrinsic :: iso_c_binding
            end subroutine otterLoopIterationBegin
        end interface
        call otterLoopIterationBegin()
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
    
    subroutine fortran_otterSynchroniseTasks_i(synchronise_descendants)
        use, intrinsic :: iso_c_binding

        ! otter_task_sync_t::otter_sync_children
        integer :: synchronise_descendants

        interface
            subroutine otterSynchroniseTasks(synchronise_descendants) bind(C, NAME="otterSynchroniseTasks")
                use, intrinsic :: iso_c_binding
                integer(c_int), value :: synchronise_descendants
            end subroutine otterSynchroniseTasks
        end interface
        call otterSynchroniseTasks(Int(synchronise_descendants, kind=c_int))
    
    end subroutine fortran_otterSynchroniseTasks_i
    
    subroutine fortran_otterSynchroniseDescendantTasksBegin_i()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterSynchroniseDescendantTasksBegin()&
                    bind(C, NAME="otterSynchroniseDescendantTasksBegin")
                use, intrinsic :: iso_c_binding
            end subroutine otterSynchroniseDescendantTasksBegin
        end interface
        call otterSynchroniseDescendantTasksBegin()
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
