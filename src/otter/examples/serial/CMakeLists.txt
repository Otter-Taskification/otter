message(STATUS "Adding serial examples")

function(add_serial_examples DIR)
    file(GLOB SERIAL_EXAMPLES  "${DIR}/*.c" "${DIR}/*.cpp" "${DIR}/*.f90" "${DIR}/*.F90")
    foreach(SRCFILE ${SERIAL_EXAMPLES})
        cmake_path(GET SRCFILE STEM EX_NAME)
        message(STATUS "Adding serial example: ${EX_NAME}")
        add_executable(${EX_NAME} ${SRCFILE})
        target_include_directories(${EX_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
        target_link_libraries(${EX_NAME} otter-serial)
        install(TARGETS ${EX_NAME})
    endforeach()
    
endfunction()

add_serial_examples(${CMAKE_CURRENT_SOURCE_DIR})
