message(STATUS "Adding examples")
message(STATUS "C Compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CXX Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Fortran Compiler ID: ${CMAKE_Fortran_COMPILER_ID}")

add_subdirectory(omp)
add_subdirectory(serial)
