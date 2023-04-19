module otter_task_graph
    enum, bind(c)
        enumerator otter_sync_children
        enumerator otter_sync_descendants
    end enum
    contains

    subroutine fortran_otterTraceInitialise()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceInitialise() bind(C, NAME="otterTraceInitialise")
                use, intrinsic :: iso_c_binding
            end subroutine otterTraceInitialise
        end interface
        call otterTraceInitialise()
    end subroutine fortran_otterTraceInitialise

    subroutine fortran_otterTraceFinalise()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceFinalise() bind(C, NAME="otterTraceFinalise")
                use, intrinsic :: iso_c_binding
            end subroutine
        end interface
        call otterTraceFinalise()
    end subroutine fortran_otterTraceFinalise

    subroutine fortran_otterTraceStart()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceStart() bind(C, NAME="otterTraceStart")
                use, intrinsic :: iso_c_binding
            end subroutine
       end interface
       call otterTraceStart()
   end subroutine fortran_otterTraceStart

   subroutine fortran_otterTraceStop()
       use, intrinsic :: iso_c_binding
       interface
           subroutine otterTraceStop() bind(C, NAME="otterTraceStop")
               use, intrinsic :: iso_c_binding
           end subroutine
       end interface
       call otterTraceStop()
   end subroutine fortran_otterTraceStop

   type(c_ptr) function fortran_otterTaskBegin_i(filename, functionname, linenum, parent_task)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: parent_task
        interface
            type(c_ptr) function otterTaskBegin(filename, functionname, linenum, parent_task) bind(C, NAME="otterTaskBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename, functionname
                integer(c_int), value :: linenum
                type(c_ptr), value :: parent_task
            end function otterTaskBegin
        end interface
        fortran_otterTaskBegin_i = otterTaskBegin(trim(filename), trim(functionname), Int(linenum, kind=c_int), parent_task)
    end function fortran_otterTaskBegin_i

    subroutine fortran_otterTaskEnd(task)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        interface
            subroutine otterTaskEnd(task) bind(C, NAME="otterTaskEnd")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
            end subroutine
        end interface
        call otterTaskEnd(task)
    end subroutine fortran_otterTaskEnd

    subroutine fortran_otterSynchroniseTasks(task, mode)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        integer :: mode
        interface
            subroutine otterSynchroniseTasks(task, mode) bind(C, NAME="otterSynchroniseTasks")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                integer(c_int), value :: mode
            end subroutine otterSynchroniseTasks
        end interface
        call otterSynchroniseTasks(task, Int(mode, kind=c_int))
    end subroutine fortran_otterSynchroniseTasks

    subroutine fortran_otterPhaseBegin(phase_name)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        interface
            subroutine otterPhaseBegin(phase_name) bind(C, NAME="otterPhaseBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
            end subroutine otterPhaseBegin
        end interface
        call otterPhaseBegin(trim(phase_name))
    end subroutine fortran_otterPhaseBegin

    subroutine fortran_otterPhaseEnd()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterPhaseEnd() bind(C, NAME="otterPhaseEnd")
                use, intrinsic :: iso_c_binding
            end subroutine
       end interface
       call otterPhaseEnd()
    end subroutine fortran_otterPhaseEnd

    subroutine fortran_otterPhaseSwitch(phase_name)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        interface
            subroutine otterPhaseSwitch(phase_name) bind(C, NAME="otterPhaseSwitch")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
            end subroutine otterPhaseSwitch
       end interface
       call otterPhaseSwitch(trim(phase_name))
   end subroutine fortran_otterPhaseSwitch
end module otter_task_graph
