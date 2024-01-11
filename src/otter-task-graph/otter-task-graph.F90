module otter_task_graph
    enum, bind(c)
        enumerator :: otter_no_add_to_pool = 0
        enumerator :: otter_add_to_pool = 1
    end enum

    enum, bind(c)
        enumerator :: otter_endpoint_enter = 0,
        enumerator :: otter_endpoint_leave = 1,
        enumerator :: otter_endpoint_discrete = 2
    end enum

    contains

    subroutine fortran_otterTraceInitialise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterTraceInitialise(source_location) bind(C, NAME="otterTraceInitialise")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: source_location
            end subroutine otterTraceInitialise
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterTraceInitialise(source_location)
    end subroutine fortran_otterTraceInitialise

    subroutine fortran_otterTraceFinalise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterTraceFinalise(source_location) bind(C, NAME="otterTraceFinalise")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value:: source_location
            end subroutine
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterTraceFinalise(source_location)
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

   type(c_ptr) function fortran_otterTaskInitialise_i(parent_task, flavour, add_to_pool, record_task_create_event, &
                                                      filename, functionname, linenum, tag)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: parent_task
        Integer :: flavour
        Integer :: add_to_pool
        Logical(c_bool) :: record_task_create_event
        character(len = *) :: tag
        type(c_ptr) :: source_location
        interface
            ! TODO
            type(c_ptr) function otterTaskInitialise(parent_task, flavour, add_to_pool, record_task_create_event, &
                                                     source_location, tag) bind(C, NAME="otterTaskInitialise")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: parent_task
                Integer(c_int), value :: flavour
                Integer(c_int), value :: add_to_pool
                Logical(c_bool), value :: record_task_create_event
                character(len=1, kind=c_char), dimension(*), intent(in) :: tag
                type(c_ptr), value :: source_location
               
            end function otterTaskInitialise
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        fortran_otterTaskInitialise_i = otterTaskInitialise(parent_task, Int(flavour, kind=c_int), Int(add_to_pool, kind=c_int),&
                                                            record_task_create_event, &
                                                            source_location, trim(tag))
    end function fortran_otterTaskInitialise_i

    subroutine fortran_otterTaskCreate(task, parent_task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: parent_task
        type(c_ptr) :: source_location
        interface
            subroutine otterTaskCreate(task, parent_task, source_location) bind(C, NAME="otterTaskCreate")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: parent_task
                type(c_ptr), value :: source_location
            end subroutine otterTaskCreate
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterTaskCreate(task, parent_task, source_location)
    end subroutine fortran_otterTaskCreate

    subroutine fortran_otterTaskStart(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: source_location
        interface
            subroutine otterTaskStart(task, start_location) bind(C, NAME="otterTaskStart")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: start_location
            end subroutine otterTaskStart
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterTaskStart(task, source_location)
    end subroutine fortran_otterTaskStart

    subroutine fortran_otterTaskEnd(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: source_location
        interface
            subroutine otterTaskEnd(task, stop_location) bind(C, NAME="otterTaskEnd")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: stop_location
            end subroutine
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterTaskEnd(task, source_location)
    end subroutine fortran_otterTaskEnd

    subroutine fortran_otterTaskPushLabel(task, label)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        character(len = *) :: label

        interface
            subroutine otterNotYetImplemented(task, label) bind(C, NAME="NYI")
                type(c_ptr), value :: task
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end subroutine
        end interface
        call otterNYI(task, trim(label))
    end subroutine fortran_otterTaskPushLabel


    function type(c_ptr) fortran_otterTaskPopLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label

        interface
            function type(c_ptr) otterNYI(label) bind(C, NAME=NYI)
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        call otterNYI(trim(label))
    end function fortran_otterTaskPopLabel


    function type(c_ptr) fortran_otterTaskBorrowLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label

        interface
            function type(c_ptr) otterNYI(label) bind(C, NAME=NYI)
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        call otterNYI(trim(label))
    end function fortran_otterTaskBorrowLabel



    subroutine fortran_otterSynchroniseTasks(task, mode, endpoint)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        integer :: mode
        integer :: endpoint
        interface
            subroutine otterSynchroniseTasks(task, mode, endpoint) bind(C, NAME="otterSynchroniseTasks")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                integer(c_int), value :: mode
                integer(c_int), value :: endpoint
            end subroutine otterSynchroniseTasks
        end interface
        call otterSynchroniseTasks(task, Int(mode, kind=c_int), Int(endpoint, kind=c_int))
    end subroutine fortran_otterSynchroniseTasks

    subroutine fortran_otterPhaseBegin(phase_name, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterPhaseBegin(phase_name, source_location) bind(C, NAME="otterPhaseBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                type(c_ptr), value :: source_location
            end subroutine otterPhaseBegin
        end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
        call otterPhaseBegin(trim(phase_name), source_location)
    end subroutine fortran_otterPhaseBegin

    subroutine fortran_otterPhaseEnd(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterPhaseEnd(source_location) bind(C, NAME="otterPhaseEnd")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: source_location
            end subroutine
       end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
       call otterPhaseEnd(source_location)
    end subroutine fortran_otterPhaseEnd

    subroutine fortran_otterPhaseSwitch(phase_name, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterPhaseSwitch(phase_name, source_location) bind(C, NAME="otterPhaseSwitch")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                type(c_ptr), value :: source_location
            end subroutine otterPhaseSwitch
       end interface
        !TODO call method to compres filename/funcanem/linenum into opaque pointer
       call otterPhaseSwitch(trim(phase_name), source_location)
   end subroutine fortran_otterPhaseSwitch
end module otter_task_graph
