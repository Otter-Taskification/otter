function(add_task_graph_examples)
    set(named_options "")
    set(one_value_keywords "")
    set(multi_value_keywords SOURCES)

    cmake_parse_arguments(PARSE_ARGV 0 TG_EXAMPLES
        "${named_options}"
        "${one_value_keywords}"
        "${multi_value_keywords}"
    )

    if(NOT TARGET otter-task-graph)
        message(FATAL_ERROR "otter-task-graph target not defined")
    endif()

    foreach(SOURCE_FILE IN LISTS TG_EXAMPLES_SOURCES)
        cmake_path(GET SOURCE_FILE STEM EXAMPLE)
        message(STATUS "Add example: ${EXAMPLE}")
        add_executable(${EXAMPLE} ${SOURCE_FILE})
        target_link_libraries(${EXAMPLE} PRIVATE otter-task-graph)
    endforeach()
endfunction()

add_task_graph_examples(SOURCES
    fibonacci.c
    task-sequences.c
)
