include(GNUInstallDirs)

# Provide the otter-task-graph library
add_library(otter-task-graph
    otter-task-graph.c
)

target_include_directories(otter-task-graph
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # for generated config.h and otter-version.h
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # for all other includes
    $<INSTALL_INTERFACE:include> # for public headers when installed
)

target_link_libraries(otter-task-graph
    PRIVATE otter-trace otter-dtype
    INTERFACE OTF2::otf2 # add to the link interface for consumers of otter-task-graph
)

target_compile_definitions(otter-task-graph PRIVATE DEBUG_LEVEL=$<IF:$<CONFIG:Debug>,3,0>)

foreach(_HEADER IN ITEMS otter-task-graph-user.h otter-task-graph.h otter-task-graph-stub.h)
    list(APPEND OTTER_TASK_GRAPH_PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/api/otter-task-graph/${_HEADER}")
endforeach()

set_target_properties(otter-task-graph PROPERTIES
    PUBLIC_HEADER
    "${OTTER_TASK_GRAPH_PUBLIC_HEADERS}"
    POSITION_INDEPENDENT_CODE ON
)

# specifies rules for installing targets from a project
install(
    TARGETS otter-task-graph
    EXPORT otter-task-graph-target
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/otter
)

# generates and installs a CMake file containing code to import targets from the
# installation tree into another project
install(EXPORT otter-task-graph-target
    FILE OtterTaskGraph.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
)
