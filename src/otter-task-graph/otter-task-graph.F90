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

    interface
        type(c_ptr) function fortran_otterCreateSourceArgs(filename, functionname, linenum) &
                                                          bind(C, NAME="otterCreateSourceArgs")
            use, intrinsic :: iso_c_binding
            character(len=1, kind=c_char), dimension(*), intent(in) :: filename
            character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
            Integer(c_int), value :: linenum
        end function fortran_otterCreateSourceArgs
    end interface

    interface
        subroutine fortran_otterFreeSourceArgs(source_args) bind(C, NAME="otterFreeSourceArgs")
            use, intrinsic :: iso_c_binding
            type(c_ptr), value :: source_args
        end subroutine fortran_otterFreeSourceArgs
    end interface

    contains

    subroutine fortran_otterTraceInitialise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterTraceInitialise(source_location) bind(C, NAME="otterTraceInitialise_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: source_location
            end subroutine otterTraceInitialise
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        call otterTraceInitialise(source_location)
        call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterTraceInitialise

    subroutine fortran_otterTraceFinalise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterTraceFinalise(source_location) bind(C, NAME="otterTraceFinalise_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value:: source_location
            end subroutine
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        call otterTraceFinalise(source_location)
        call fortran_otterFreeSourceArgs(source_location)
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

   type(c_ptr) function fortran_otterTaskInitialise(parent_task, flavour, add_to_pool, record_task_create_event, &
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
                                                     source_location, tag) bind(C, NAME="otterTaskInitialise_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: parent_task
                Integer(c_int), value :: flavour
                Integer(c_int), value :: add_to_pool
                Logical(c_bool), value :: record_task_create_event
                character(len=1, kind=c_char), dimension(*), intent(in) :: tag
                type(c_ptr), value :: source_location
               
            end function otterTaskInitialise
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        fortran_otterTaskInitialise_i = otterTaskInitialise(parent_task, Int(flavour, kind=c_int), Int(add_to_pool, kind=c_int),&
                                                            record_task_create_event, &
                                                            source_location, trim(tag))
        call fortran_otterFreeSourceArgs(source_location)
    end function fortran_otterTaskInitialise

    subroutine fortran_otterTaskCreate(task, parent_task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: parent_task
        type(c_ptr) :: source_location
        interface
            subroutine otterTaskCreate(task, parent_task, source_location) bind(C, NAME="otterTaskCreate_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: parent_task
                type(c_ptr), value :: source_location
            end subroutine otterTaskCreate
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        call otterTaskCreate(task, parent_task, source_location)
        call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterTaskCreate

    type(c_ptr) function fortran_otterTaskStart(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: source_location
        interface
            type(c_ptr) function otterTaskStart(task, start_location) bind(C, NAME="otterTaskStart_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: start_location
            end subroutine otterTaskStart
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        fortran_otterTaskStart = otterTaskStart(task, source_location)
        call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterTaskStart

    subroutine fortran_otterTaskEnd(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: source_location
        interface
            subroutine otterTaskEnd(task, stop_location) bind(C, NAME="otterTaskEnd_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: stop_location
            end subroutine
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        call otterTaskEnd(task, source_location)
        call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterTaskEnd

    subroutine fortran_otterTaskPushLabel(task, label)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        character(len = *) :: label
        interface
            subroutine otterTaskPushLabel(task, label) bind(C, NAME="otterTaskPushLabel_f")
                type(c_ptr), value :: task
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end subroutine
        end interface
        call ottertaskPushLabel(task, trim(label))
    end subroutine fortran_otterTaskPushLabel


    type(c_ptr) function fortran_otterTaskPopLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label

        interface
            function type(c_ptr) otterTaskPopLabel(label) bind(C, NAME="otterTaskPopLabel_f")
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        fortran_otterTaskPopLabel = otterTaskPopLabel(trim(label))
    end function fortran_otterTaskPopLabel


    type(c_ptr) function fortran_otterTaskBorrowLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label
        interface
            function type(c_ptr) otterTaskBorrowLabel(label) bind(C, NAME="otterTaskBorrowLabel_f")
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        fortran_otterTaskBorrowLabel = otterTaskBorrowLabel(trim(label))
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
            subroutine otterPhaseBegin(phase_name, source_location) bind(C, NAME="otterPhaseBegin_f")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                type(c_ptr), value :: source_location
            end subroutine otterPhaseBegin
        end interface
        source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
        call otterPhaseBegin(trim(phase_name), source_location)
        call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterPhaseBegin

    subroutine fortran_otterPhaseEnd(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterPhaseEnd(source_location) bind(C, NAME="otterPhaseEnd_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: source_location
            end subroutine
       end interface
       source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
       call otterPhaseEnd(source_location)
       call fortran_otterFreeSourceArgs(source_location)
    end subroutine fortran_otterPhaseEnd

    subroutine fortran_otterPhaseSwitch(phase_name, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: source_location
        interface
            subroutine otterPhaseSwitch(phase_name, source_location) bind(C, NAME="otterPhaseSwitch_f")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                type(c_ptr), value :: source_location
            end subroutine otterPhaseSwitch
       end interface
       source_location = fortran_otterCreateSourceArgs(trim(filename), trim(functionname), Int(linenum, Kind=c_int))
       call otterPhaseSwitch(trim(phase_name), source_location)
       call fortran_otterFreeSourceArgs(source_location)
   end subroutine fortran_otterPhaseSwitch
end module otter_task_graph
