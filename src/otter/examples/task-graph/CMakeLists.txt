message(STATUS "Adding task-graph examples")

function(add_task_graph_examples DIR)
    file(GLOB SERIAL_EXAMPLES  "${DIR}/*.c" "${DIR}/*.cpp" "${DIR}/*.f90")
    foreach(SRCFILE ${SERIAL_EXAMPLES})
        cmake_path(GET SRCFILE STEM EX_NAME)
        message(STATUS "Adding task-graph example: ${EX_NAME}")
        add_executable(${EX_NAME} ${SRCFILE})
        target_include_directories(${EX_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
        target_link_libraries(${EX_NAME} otter-task-graph)
        install(TARGETS ${EX_NAME})
    endforeach()
    
endfunction()

add_task_graph_examples(${CMAKE_CURRENT_SOURCE_DIR})
