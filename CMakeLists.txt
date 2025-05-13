# 3.19: presets
# 3.20: buildPresets
# 3.21: $<TARGET_OBJECTS>
# 3.23: CMakePresets.json version 4 (use includes)
cmake_minimum_required(VERSION 3.23)

project(Otter
    VERSION 0.1.0
    HOMEPAGE_URL https://github.com/Otter-Taskification/otter
    LANGUAGES C CXX Fortran
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(ConfigurePackageModulefile)

option(WITH_EXAMPLES "Generate and build examples for demonstrating Otter")
option(WITH_TESTS "Generate and build tests")
option(WITH_OMPT_PLUGIN "Build the OMPT plugin")
option(BUILD_SHARED_LIBS "Build shared libraries")

# Set output locations within the build tree
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/lib/fmod)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ar)

# Custom Info build type is the same as Debug, but uses a different logging level
set(CMAKE_C_FLAGS_INFO "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_INFO "${CMAKE_CXX_FLAGS_DEBUG}")

# Set OTF2_ROOT empty in the cache, the user has to provide a value
set(OTF2_ROOT "" CACHE FILEPATH "The root of the OTF2 installation")
find_package(OTF2)

if(NOT OTF2_FOUND)
    message(FATAL_ERROR "OTF2 was not found. Did you specify OTF2_ROOT?")
else()
    message(VERBOSE "OTF2_ROOT: ${OTF2_ROOT}")
endif()

# Configure version header
configure_file(
    ${PROJECT_SOURCE_DIR}/include/public/otter-version.h.in
    ${PROJECT_BINARY_DIR}/include/public/otter-version.h # write to build tree
)

# Configure config/.h
configure_file(
    ${PROJECT_SOURCE_DIR}/include/public/config.h.in
    ${PROJECT_BINARY_DIR}/include/public/config.h # write to build tree
)

add_library(Otter::debug_level IMPORTED INTERFACE)
target_compile_definitions(Otter::debug_level INTERFACE
    $<IF:$<CONFIG:Debug>, DEBUG_LEVEL=3,
    $<IF:$<CONFIG:Info>, DEBUG_LEVEL=2,
    $<IF:$<CONFIG:Release>, DEBUG_LEVEL=1,
    $<IF:$<CONFIG:RelWithDebInfo>, DEBUG_LEVEL=1, DEBUG_LEVEL=0>>>>
)

add_subdirectory(src/types)
add_subdirectory(src/otter-trace)
add_subdirectory(src/otter-task-graph)

if(WITH_OMPT_PLUGIN)
    message(STATUS "ENABLED: OMPT plugin")
    add_subdirectory(src/otter-ompt)
else()
    message(STATUS "DISABLED: OMPT plugin")
endif()

if(WITH_TESTS)
    message(STATUS "ENABLED: tests")
    add_subdirectory(test)
else()
    message(STATUS "DISABLED: tests")
endif()

if(WITH_EXAMPLES)
    message(STATUS "ENABLED: examples")
    add_subdirectory(examples)
else()
    message(STATUS "DISABLED: examples")
endif()

include(cmake/GenerateOtterPackageConfig.cmake)

# install the FindOTF2.cmake script for use by consumers of Otter
install(FILES
    ${PROJECT_SOURCE_DIR}/cmake/FindOTF2.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
)

# create a modulefile which can detect the root of its installation tree
configure_package_modulefile(
    ${PROJECT_SOURCE_DIR}/src/modulefiles/otter.in
    ${PROJECT_BINARY_DIR}/src/modulefiles/otter
    INSTALL_DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/modulefiles/otter
)

install(FILES
    ${PROJECT_BINARY_DIR}/src/modulefiles/otter
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/modulefiles/otter
)