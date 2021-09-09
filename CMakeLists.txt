# Main CMakeLists.txt for Otter

execute_process (
    COMMAND bash -c "${CMAKE_C_COMPILER} --version | head -n 1" 
    OUTPUT_VARIABLE CC_VERSION)
message("CC_VERSION=${CC_VERSION}")

include(CheckSymbolExists)
include(CheckIncludeFile)

cmake_minimum_required(VERSION 3.10)

project(Otter VERSION 0.1.0)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Define target library and source files
add_library(otter 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/dt-queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/dt-stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-entry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-structs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-structs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/callback.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/datatypes-common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/general.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/ompt-callback-prototypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-entry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-environment-variables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-ompt-header.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-structs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/stack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/trace-attribute-defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/trace-attributes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/trace.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/trace-lookup-macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/trace-structs.h
)

# Specify Otter install location
install(TARGETS otter DESTINATION lib)

# Configure version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h
)

# Headers required to compile Otter
target_include_directories(otter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link to dependencies
target_link_libraries(otter pthread)

# # # OMPT # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Check for omp-tools.h
CHECK_INCLUDE_FILE(omp-tools.h HAVE_OMP_TOOLS_H)
if(NOT HAVE_OMP_TOOLS_H)
    message(FATAL_ERROR "\tCan't find omp-tools.h - did you specify an OMPT-supporting compiler?")
endif()
# Check for symbol in omp-tools.h - more recent compilers come with 5.1 versions
# of some OMPT constructs e.g. master -> masked.
check_symbol_exists(DEPRECATED_51 omp-tools.h HAVE_DEPRECATED_51)
if(HAVE_DEPRECATED_51)
    message("++ Using 5.1")
    add_compile_definitions(USE_OMPT_MASKED)
else()
    message("++ Not using 5.1")
endif()

# # # OTF2 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add OTF2 include directory if it was specified
set(HAVE_OTF2_INCLUDE_DIR FALSE)
if(DEFINED OTF2_INCLUDE_DIR)
    target_include_directories(otter PRIVATE ${OTF2_INCLUDE_DIR})
    set(HAVE_OTF2_INCLUDE_DIR TRUE)
    message("++ OTF2 include dir was specified")
endif()

target_link_libraries(otter otf2)
# Add library directory for OTF2 if it was specified
if(DEFINED OTF2_LIB_DIR)
    target_link_libraries(otter -L${OTF2_LIB_DIR})
endif()

# Check for otf2.h
set(CMAKE_REQUIRED_INCLUDES $<$<BOOL:HAVE_OTF2_INCLUDE_DIR>:${OTF2_INCLUDE_DIR}>)
CHECK_INCLUDE_FILE(otf2/otf2.h HAVE_OTF2_H)
if(NOT HAVE_OTF2_H)
    message(FATAL_ERROR "Can't find otf2.h - did you specify OTF2_INCLUDE_DIR?")
endif()
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #