module otter_task_graph

    contains

     enum, bind(c)
         ENUMERATOR otter_sync_children
         ENUMERATOR otter_sync_descendants
     end enum

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
    end subroutine fortran_otterTraceFinalise()

    subroutine fortran_otterTraceStart()
        use, intrinsic :: iso_c_binding
        interface
            subroutine otterTraceStart() bind(C, NAME="otterTraceStart")
                use, intrinsic :: iso_c_binding
            end subroutine
       end interface
       call otterTraceStart()
   end subroutine fortran_otterTraceStart()

   subroutine fortran_otterTraceStop()
       use, intrinsic :: iso_c_binding
       interface
           subroutine otterTraceStop() bind(C, NAME="otterTraceStop")
               use, intrinsic :: iso_c_binding
           end subroutine
       end interface
       call otterTraceStop()
   end subroutine fortran_otterTraceStop()

    function fortran_otterTaskBegin_i(filename, functionname, linenum, parent_task)
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
                type(c_ptr) :: parent_task
            end subroutine otterTaskBegin
        end interface
        fortran_otterTaskBegin_i = otterTaskBegin(trim(filename), trim(functionname), Int(linenum, kind=c_int), parent_task)
    end function fortran_otterTaskBegin_i

    subroutine fortran_otterTaskEnd(task)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        interface
            subroutine otterTaskEnd(task) bind(C, NAME="otterTaskEnd")
                use, intrinsic :: iso_c_binding
                type(c_ptr) :: task
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
                type(c_ptr) :: task
                integer(c_int), value :: mode
            end subroutine otterSynchroniseTasks
        end interface
        call otterSynchroniseTasks(task, Int(mode, kind=c_int))
    end subroutine

    subroutine fortran_otterPhaseBegin(_name)
        use, intrinsic :: iso_c_binding
        character(len = *) :: _name
        interface
            subroutine otterPhaseBegin(_name) bind(C, NAME="otterPhaseBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: _name
            end subroutine otterPhaseBegin
        end interface
        call otterPhaseBegin(trim(_name))
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

    subroutine fortran_otterPhaseSwitch(_name)
        use, intrinsic :: iso_c_binding
        character(len = *) :: _name
        interface
            subroutine otterPhaseSwitch(_name) bind(C, NAME="otterPhaseSwitch")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: _name
            end subroutine otterPhaseSwitch
       end interface
       call otterPhaseSwitch(trim(_name))
   end subroutine fortran_otterPhaseSwitch
end module otter_task_graph
