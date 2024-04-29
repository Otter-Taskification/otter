include(GNUInstallDirs)

# Provide the otter-trace target
add_library(otter-trace OBJECT
    trace-ompt.c
    trace-task-graph.c
    trace-location.c
    trace-region-def.c
    trace-archive.c
    trace-initialise.c
    trace-unique-refs.c
    trace-thread-data.c
    trace-task-data.c
    trace-parallel-data.c
    trace-task-context.c
    source-location.c
    strings.c
    trace-task-manager.c
)

target_include_directories(otter-trace
    PRIVATE ${PROJECT_BINARY_DIR}/include # for config.h and otter-version.h
    PRIVATE ${PROJECT_SOURCE_DIR}/include # for all other includes
)

target_link_libraries(otter-trace
    PRIVATE Otter::debug_level
    PUBLIC OTF2::otf2 # some otter-trace public headers include OTF2
)

set_property(TARGET otter-trace PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS otter-trace
    EXPORT otter-trace-target
)

install(EXPORT otter-trace-target
    FILE OtterTrace.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
)
