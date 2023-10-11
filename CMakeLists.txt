# 3.19: presets
# 3.20: buildPresets
# 3.21: $<TARGET_OBJECTS>
cmake_minimum_required(VERSION 3.21)

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

# Absolutely required
find_package(OTF2)
message(VERBOSE "OTF2_ROOT: ${OTF2_ROOT}")

if(NOT OTF2_FOUND)
    message(FATAL_ERROR "OTF2 was not found. Did you specify OTF2_INSTALL_DIR?")
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

add_subdirectory(src/types)
add_subdirectory(src/otter-trace)
add_subdirectory(src/otter-task-graph)

if(WITH_OMPT_PLUGIN)
    message(STATUS "Enable OMPT plugin")
    add_subdirectory(src/otter-ompt)
endif()

if(WITH_TESTS)
    message(STATUS "Enable tests")
    add_subdirectory(test)
endif()

if(WITH_EXAMPLES)
    message(STATUS "Enable examples")
    add_subdirectory(examples)
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