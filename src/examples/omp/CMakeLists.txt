message(STATUS "Adding OpenMP examples")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" 
    OR CMAKE_Fortran_COMPILER_ID STREQUAL "GNU" 
    OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "It is not recommended to compile OpenMP examples with GNU compilers. Please use LLVM or Intel compilers.")
endif()

# Add -[f|q]openmp flag"
include(FindOpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${OpenMP_Fortran_FLAGS}")

message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message(STATUS "OpenMP_CXX_FLAGS=${OpenMP_CXX_FLAGS}")
message(STATUS "OpenMP_Fortran_FLAGS=${OpenMP_Fortran_FLAGS}")

function(add_omp_examples DIR)
    file(GLOB SERIAL_EXAMPLES  "${DIR}/*.c" "${DIR}/*.cpp" "${DIR}/*.f90")
    foreach(SRCFILE ${SERIAL_EXAMPLES})
        cmake_path(GET SRCFILE STEM EX_NAME)
        set(EX_NAME otter-${EX_NAME})
        message(STATUS "Adding OpenMP example: ${EX_NAME}")
        add_executable(${EX_NAME} ${SRCFILE})
        install(TARGETS ${EX_NAME})
    endforeach()
    
endfunction()

add_omp_examples(${CMAKE_CURRENT_SOURCE_DIR})
