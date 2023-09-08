include(GNUInstallDirs)

# Provide the otter-types target
add_library(otter-dtype OBJECT
    dt-queue.c
    dt-stack.c
    string_value_registry.cpp
    vptr_manager.cpp
)

target_include_directories(otter-dtype
    PRIVATE ${PROJECT_BINARY_DIR}/include # for generated config.h and otter-version.h
    PRIVATE ${PROJECT_SOURCE_DIR}/include # for all other includes
)

target_compile_definitions(otter-dtype PRIVATE DEBUG_LEVEL=$<IF:$<CONFIG:Verbose>,3,0>)

set_property(TARGET otter-dtype PROPERTY POSITION_INDEPENDENT_CODE ON)

# an object library with no install destination doesn't install files, but this
# is required to export otter-dtype. The exported object library becomes an
# interface library
install(TARGETS otter-dtype
    EXPORT otter-dtype-target
)

install(EXPORT otter-dtype-target
    FILE OtterDType.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
)
