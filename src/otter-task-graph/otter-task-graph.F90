module otter_task_graph
    enum, bind(c)
        enumerator :: otter_no_add_to_pool
        enumerator :: otter_add_to_pool
    end enum

    enum, bind(c)
        enumerator :: otter_endpoint_enter
        enumerator :: otter_endpoint_leave
        enumerator :: otter_endpoint_discrete
    end enum

    public
    contains

    subroutine fortran_otterTraceInitialise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTraceInitialise(filename, functionname, linenum) bind(C, NAME="otterTraceInitialise")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine otterTraceInitialise
        end interface
        call otterTraceInitialise(trim(filename) // c_null_char, trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterTraceInitialise

    subroutine fortran_otterTraceFinalise(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterTraceFinalise(filename, functionname, linenum) bind(C, NAME="otterTraceFinalise")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine
        end interface
        call otterTraceFinalise(trim(filename) // c_null_char, trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
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
        interface
            ! TODO
            type(c_ptr) function otterTaskInitialise(parent_task, flavour, add_to_pool, record_task_create_event, &
            filename, functionname, linenum, tag) bind(C, NAME="otterTaskInitialise_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: parent_task
                Integer(c_int), value :: flavour
                Integer(c_int), value :: add_to_pool
                Logical(c_bool), value :: record_task_create_event
                character(len=1, kind=c_char), dimension(*), intent(in) :: tag
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
               
            end function otterTaskInitialise
        end interface
        fortran_otterTaskInitialise = otterTaskInitialise(parent_task, Int(flavour, kind=c_int), Int(add_to_pool, kind=c_int),&
                                                            record_task_create_event, &
                                                            trim(filename) // c_null_char, &
                                                            trim(functionname) // c_null_char &
                                                            , Int(linenum, Kind=c_int), trim(tag)  // c_null_char)
    end function fortran_otterTaskInitialise

    subroutine fortran_otterTaskCreate(task, parent_task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        type(c_ptr) :: parent_task
        interface
            subroutine otterTaskCreate(task, parent_task, filename, functionname, linenum) bind(C, NAME="otterTaskCreate")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                type(c_ptr), value :: parent_task
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine otterTaskCreate
        end interface
        call otterTaskCreate(task, parent_task, trim(filename) // c_null_char, &
            trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterTaskCreate

    type(c_ptr) function fortran_otterTaskStart(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        interface
            type(c_ptr) function otterTaskStart(task, filename, functionname, linenum) bind(C, NAME="otterTaskStart")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end function otterTaskStart
        end interface
        fortran_otterTaskStart = otterTaskStart(task, trim(filename) // c_null_char, &
            trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end function fortran_otterTaskStart

    subroutine fortran_otterTaskEnd(task, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        type(c_ptr) :: task
        interface
            subroutine otterTaskEnd(task, filename, functionname, linenum) bind(C, NAME="otterTaskEnd")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine
        end interface
        call otterTaskEnd(task, trim(filename)  // c_null_char, &
            trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterTaskEnd

    subroutine fortran_otterTaskPushLabel(task, label)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        character(len = *) :: label
        interface
            subroutine otterTaskPushLabel(task, label) bind(C, NAME="otterTaskPushLabel_f")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end subroutine
        end interface
        call ottertaskPushLabel(task, trim(label) // c_null_char)
    end subroutine fortran_otterTaskPushLabel


    type(c_ptr) function fortran_otterTaskPopLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label

        interface
            type(c_ptr) function otterTaskPopLabel(label) bind(C, NAME="otterTaskPopLabel_f")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        fortran_otterTaskPopLabel = otterTaskPopLabel(trim(label) // c_null_char)
    end function fortran_otterTaskPopLabel


    type(c_ptr) function fortran_otterTaskBorrowLabel(label)
        use, intrinsic :: iso_c_binding
        character(len = *) :: label
        interface
            type(c_ptr) function otterTaskBorrowLabel(label) bind(C, NAME="otterTaskBorrowLabel_f")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: label
            end function
        end interface
        fortran_otterTaskBorrowLabel = otterTaskBorrowLabel(trim(label)  // c_null_char)
    end function fortran_otterTaskBorrowLabel



    subroutine fortran_otterSynchroniseTasks(task, mode, endpoint, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        type(c_ptr) :: task
        integer :: mode
        integer :: endpoint
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterSynchroniseTasks(task, mode, endpoint, filename, functionname, linenum) bind(C, NAME="otterSynchroniseTasks")
                use, intrinsic :: iso_c_binding
                type(c_ptr), value :: task
                integer(c_int), value :: mode
                integer(c_int), value :: endpoint
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine otterSynchroniseTasks
        end interface
        call otterSynchroniseTasks(task, Int(mode, kind=c_int), &
            Int(endpoint, kind=c_int), trim(filename)  // c_null_char, &
            trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterSynchroniseTasks

    subroutine fortran_otterPhaseBegin(phase_name, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterPhaseBegin(phase_name, filename, functionname, linenum) bind(C, NAME="otterPhaseBegin")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine otterPhaseBegin
        end interface
        call otterPhaseBegin(trim(phase_name) // c_null_char, trim(filename) // c_null_char, &
            trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterPhaseBegin

    subroutine fortran_otterPhaseEnd(filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterPhaseEnd(filename, functionname, linenum) bind(C, NAME="otterPhaseEnd")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine
       end interface
       call otterPhaseEnd(trim(filename) // c_null_char, trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
    end subroutine fortran_otterPhaseEnd

    subroutine fortran_otterPhaseSwitch(phase_name, filename, functionname, linenum)
        use, intrinsic :: iso_c_binding
        character(len = *) :: phase_name
        character(len = *) :: filename
        character(len = *) :: functionname
        integer :: linenum
        interface
            subroutine otterPhaseSwitch(phase_name, filename, functionname, linenum) bind(C, NAME="otterPhaseSwitch")
                use, intrinsic :: iso_c_binding
                character(len=1, kind=c_char), dimension(*), intent(in) :: phase_name
                character(len=1, kind=c_char), dimension(*), intent(in) :: filename
                character(len=1, kind=c_char), dimension(*), intent(in) :: functionname
                Integer(c_int), value :: linenum
            end subroutine otterPhaseSwitch
       end interface
       call otterPhaseSwitch(trim(phase_name) // c_null_char, trim(filename) // c_null_char,&
           trim(functionname) // c_null_char, Int(linenum, Kind=c_int))
   end subroutine fortran_otterPhaseSwitch
end module otter_task_graph
