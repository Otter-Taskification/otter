message("C Compiler ID: ${CMAKE_C_COMPILER_ID}")
message("CXX Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Fortran Compiler ID: ${CMAKE_Fortran_COMPILER_ID}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" 
    OR CMAKE_Fortran_COMPILER_ID STREQUAL "GNU" 
    OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "It is not recommended to compile examples with GNU compilers. Please use LLVM or Intel compilers.")
endif()

# Add -[f|q]openmp flag"
include(FindOpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${OpenMP_Fortran_FLAGS}")

message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message(STATUS "OpenMP_CXX_FLAGS=${OpenMP_CXX_FLAGS}")
message(STATUS "OpenMP_Fortran_FLAGS=${OpenMP_Fortran_FLAGS}")

# Add C executables
add_executable(omp-demo "omp-demo.c")
add_executable(omp-fibonacci "omp-fibonacci.c")
add_executable(omp-getcpu "omp-getcpu.c")
add_executable(omp-master "omp-master.c")
add_executable(omp-parallel-2 "omp-parallel-2.c")
add_executable(omp-parallel "omp-parallel.c")
add_executable(omp-parallel-for "omp-parallel-for.c")
add_executable(omp-parallel-single "omp-parallel-single.c")
add_executable(omp-parallel-single-nowait "omp-parallel-single-nowait.c")
add_executable(omp-parallel-for-task "omp-parallel-for-task.c")
add_executable(omp-parallel-if "omp-parallel-if.c")
add_executable(omp-parallel-nested "omp-parallel-nested.c")
add_executable(omp-parallel-nested-multi "omp-parallel-nested-multi.c")
add_executable(omp-parallel-nested-single "omp-parallel-nested-single.c")
add_executable(omp-parallel-nested-taskloop "omp-parallel-nested-taskloop.c")
add_executable(omp-taskgroup "omp-taskgroup.c")
add_executable(omp-taskgroup-single "omp-taskgroup-single.c")
add_executable(omp-taskloop "omp-taskloop.c")
add_executable(omp-taskloop-nogroup "omp-taskloop-nogroup.c")
add_executable(omp-taskloop-nogroup-single "omp-taskloop-nogroup-single.c")
add_executable(omp-taskloop-single "omp-taskloop-single.c")
add_executable(omp-taskloop-yield "omp-taskloop-yield.c")
add_executable(omp-spawn-n-tasks "omp-spawn-n-tasks.c")
add_executable(omp-spawn-n-tasks-in-explicit-task-with-yield "omp-spawn-n-tasks-in-explicit-task-with-yield.c")

add_executable(otter-serial-fib
    "otter-serial-fib.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/otter/otter-serial.h")
target_include_directories(otter-serial-fib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
target_link_libraries(otter-serial-fib otter-serial)

add_executable(otter-serial-loop
    "otter-serial-loop.c")
target_include_directories(otter-serial-loop PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
target_link_libraries(otter-serial-loop otter-serial)

add_executable(otter-serial-taskwait-some
    "otter-serial-taskwait-some.c")
target_include_directories(otter-serial-taskwait-some PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
target_link_libraries(otter-serial-taskwait-some otter-serial)

# Add CXX executables
add_executable(omp-peano-constructs "omp-peano-constructs.cpp")

# Add Fortran executables
add_executable(omp-parallel-single-for-fortran "omp-parallel-single-for-fortran.f90")
# set_property(TARGET omp-parallel-single-for-fortran PROPERTY LINKER_LANGUAGE Fortran)