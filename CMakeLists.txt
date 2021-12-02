# Main CMakeLists.txt for Otter

cmake_minimum_required(VERSION 3.12)

include(CheckSymbolExists)
include(CheckIncludeFile)

### Declare the main project
project(Otter
    VERSION 0.1.0
    HOMEPAGE_URL https://github.com/adamtuft/otter
    LANGUAGES C CXX Fortran
)

### Define key settings
option(WITH_EXAMPLES "Generate and build examples for demonstrating Otter" OFF)
option(WITH_TESTS "Generate and build tests" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(OTF2_INCLUDE_DIR "/opt/otf2/include" CACHE STRING "OTF2 include directory")
set(OTF2_LIB_DIR "/opt/otf2/lib" CACHE STRING "OTF2 library directory")

if (CMAKE_C_FLAGS)
    message(STATUS "Flags added: \"${CMAKE_C_FLAGS}\"")
endif()

# Select default config type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build type: Debug (default)")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

# Get debug levels if specified
if (NOT OTTER_DEBUG_LEVELS)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Debug levels: 3,0,0 (default)")
        set(OTTER_DEBUG_LEVELS "3,0,0" CACHE STRING "Debug levels for Otter components (default=3,0,0)" FORCE)
    else()
        message(STATUS "Debug levels: 0,0,0 (default)")
        set(OTTER_DEBUG_LEVELS "0,0,0" CACHE STRING "Debug levels for Otter components (default=0,0,0)" FORCE)
    endif()
else()
    set(OTTER_DEBUG_LEVELS "${OTTER_DEBUG_LEVELS}" CACHE STRING "Debug levels for Otter components" FORCE)
endif()

# Parse debug levels into individual values
string(REPLACE "," ";" OTTER_DEBUG_LEVELS_LIST "${OTTER_DEBUG_LEVELS}")
list(LENGTH OTTER_DEBUG_LEVELS_LIST OTTER_DEBUG_LEVELS_LIST_LEN)
if(NOT OTTER_DEBUG_LEVELS_LIST_LEN EQUAL "3")
    message(FATAL_ERROR "Invalid debug level list: ${OTTER_DEBUG_LEVELS}")
endif()

list(GET OTTER_DEBUG_LEVELS_LIST 0 OTTER_DEBUG_CORE)
if (NOT OTTER_DEBUG_CORE MATCHES "^[0-9]+$")
    message(FATAL_ERROR "Invalid debug level: ${OTTER_DEBUG_CORE}")
endif()

list(GET OTTER_DEBUG_LEVELS_LIST 1 OTTER_DEBUG_TRACE)
if (NOT OTTER_DEBUG_TRACE MATCHES "^[0-9]+$")
    message(FATAL_ERROR "Invalid debug level: ${OTTER_DEBUG_TRACE}")
endif()

list(GET OTTER_DEBUG_LEVELS_LIST 2 OTTER_DEBUG_DTYPE)
if (NOT OTTER_DEBUG_DTYPE MATCHES "^[0-9]+$")
    message(FATAL_ERROR "Invalid debug level: ${OTTER_DEBUG_DTYPE}")
endif()

# Extract C compiler version as separate components
execute_process (
    COMMAND bash -c "${CMAKE_C_COMPILER} -dumpversion | tr -d '\n'  | sed 's/\\./\\;/g'" 
    OUTPUT_VARIABLE C_COMPILER_VERSION_LIST
)
list(GET C_COMPILER_VERSION_LIST 0 C_COMPILER_VERSION_MAJOR)
list(GET C_COMPILER_VERSION_LIST 1 C_COMPILER_VERSION_MINOR)
list(GET C_COMPILER_VERSION_LIST 2 C_COMPILER_VERSION_BUGFIX)

# Build version string component-wise
set(C_COMPILER_VERSION ${C_COMPILER_VERSION_MAJOR})
if(NOT "${C_COMPILER_VERSION_MINOR}" STREQUAL "")
    set(C_COMPILER_VERSION "${C_COMPILER_VERSION}.${C_COMPILER_VERSION_MINOR}")
    if(NOT "${C_COMPILER_VERSION_BUGFIX}" STREQUAL "")
        set(C_COMPILER_VERSION "${C_COMPILER_VERSION}.${C_COMPILER_VERSION_BUGFIX}")
    endif()
endif()

message(STATUS "C compiler selected: ${CMAKE_C_COMPILER_ID} ${C_COMPILER_VERSION}")

# Detect requirement to use 5.1 features instead of deprecated 5.1 features
set(USE_OMPT_51 0)
set(USE_OMPT_51_PARTIAL 0)
if(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    if("${C_COMPILER_VERSION}" VERSION_GREATER "2021.2")
        message(STATUS "Detected Intel >2021.2")
        set(USE_OMPT_51 1)
    elseif("${C_COMPILER_VERSION}" VERSION_LESS_EQUAL "2021.2")
        # icc 2021.2 omp-tools.h isn't consistent with 5.0->5.1 deprecated features
        message(STATUS "Detected Intel <=2021.2")
        message(STATUS "Must use OMPT 5.0/5.1 combined")
        set(USE_OMPT_51 1)
        set(USE_OMPT_51_PARTIAL 1)
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    if("${C_COMPILER_VERSION}" VERSION_GREATER "11.1")
        message(STATUS "Detected Clang >11")
        set(USE_OMPT_51 1)
    endif()
else()
    message(WARN "Selected C compiler may not be supported")
endif()

# Define library components
add_library(otter-core OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-entry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-structs.c
)
target_include_directories(otter-core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(otter-core BEFORE PRIVATE ${CMAKE_C_FLAGS})
target_compile_definitions(otter-core PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_CORE})
set_property(TARGET otter-core PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(otter-trace OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-location.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-region-parallel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-region-workshare.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-region-master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-region-sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-region-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace-archive.c
)
target_include_directories(otter-trace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(otter-trace PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_TRACE})
set_property(TARGET otter-trace PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(otter-dtype OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/dt-queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/dt-stack.c
)
target_include_directories(otter-dtype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(otter-dtype PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_DTYPE})
set_property(TARGET otter-dtype PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(otter-serial-events OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/otter-serial.c
)
target_include_directories(otter-serial-events PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(otter-serial-events BEFORE PRIVATE ${CMAKE_C_FLAGS})
target_compile_definitions(otter-serial-events PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_CORE})
set_property(TARGET otter-serial-events PROPERTY POSITION_INDEPENDENT_CODE ON)

# Link component objects into shared library
add_library(otter
    $<TARGET_OBJECTS:otter-core>
    $<TARGET_OBJECTS:otter-trace>
    $<TARGET_OBJECTS:otter-dtype>
)

add_library(otter-serial
    $<TARGET_OBJECTS:otter-serial-events>
    $<TARGET_OBJECTS:otter-trace>
    $<TARGET_OBJECTS:otter-dtype>
)

# Specify Otter install location
install(TARGETS otter DESTINATION lib)
install(TARGETS otter-serial DESTINATION lib)

# Configure version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h
)

# Link to dependencies
target_link_libraries(otter pthread)
target_link_libraries(otter-serial pthread)

# # # OMPT # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Check for omp-tools.h
CHECK_INCLUDE_FILE(omp-tools.h HAVE_OMP_TOOLS_H)
if(NOT HAVE_OMP_TOOLS_H)
    message(FATAL_ERROR "Can't find omp-tools.h - did you specify an OMPT-supporting compiler?")
endif()

# Flag that we should use OMPT 5.1 features
if(USE_OMPT_51)
    message(STATUS "Using OMPT 5.1")
    target_compile_definitions(otter-core PRIVATE USE_OMPT_MASKED)
    target_compile_definitions(otter-trace PRIVATE USE_OMPT_MASKED)
else()
    message(STATUS "Not using OMPT 5.1")
endif()

# # # OTF2 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add OTF2 include directory if it was specified
set(HAVE_OTF2_INCLUDE_DIR FALSE)
if(DEFINED OTF2_INCLUDE_DIR)
    target_include_directories(otter-core PRIVATE ${OTF2_INCLUDE_DIR})
    target_include_directories(otter-serial-events PRIVATE ${OTF2_INCLUDE_DIR})
    target_include_directories(otter-trace PRIVATE ${OTF2_INCLUDE_DIR})
    set(HAVE_OTF2_INCLUDE_DIR TRUE)
    message(STATUS "OTF2 include dir was specified")
endif()

target_link_libraries(otter otf2)
target_link_libraries(otter-serial otf2)
# Add library directory for OTF2 if it was specified
if(DEFINED OTF2_LIB_DIR)
    target_link_libraries(otter -L${OTF2_LIB_DIR})
    target_link_libraries(otter-serial -L${OTF2_LIB_DIR})
endif()

# Check for otf2.h
set(CMAKE_REQUIRED_INCLUDES $<$<BOOL:HAVE_OTF2_INCLUDE_DIR>:${OTF2_INCLUDE_DIR}>)
CHECK_INCLUDE_FILE(otf2/otf2.h HAVE_OTF2_H)
if(NOT HAVE_OTF2_H)
    message(FATAL_ERROR "Can't find otf2.h - did you specify OTF2_INCLUDE_DIR?")
endif()

# Check OTF2 version found
file(READ "${OTF2_INCLUDE_DIR}/otf2/OTF2_GeneralDefinitions.h" otf2_defs_h)

string(REGEX MATCH "#define OTF2_VERSION_MAJOR[ \t]+[0-9]+" OTF2_VERSION_MAJOR_DEF "${otf2_defs_h}")
string(REGEX REPLACE "#define OTF2" "" OTF2_VERSION_MAJOR_R "${OTF2_VERSION_MAJOR_DEF}")
string(REGEX MATCH "[0-9]+" OTF2_VERSION_MAJOR "${OTF2_VERSION_MAJOR_R}")

string(REGEX MATCH "#define OTF2_VERSION_MINOR[ \t]+[0-9]+" OTF2_VERSION_MINOR_DEF "${otf2_defs_h}")
string(REGEX REPLACE "#define OTF2" "" OTF2_VERSION_MINOR_R "${OTF2_VERSION_MINOR_DEF}")
string(REGEX MATCH "[0-9]+" OTF2_VERSION_MINOR "${OTF2_VERSION_MINOR_R}")

string(REGEX MATCH "#define OTF2_VERSION_BUGFIX[ \t]+[0-9]+" OTF2_VERSION_BUGFIX_DEF "${otf2_defs_h}")
string(REGEX REPLACE "#define OTF2" "" OTF2_VERSION_BUGFIX_R "${OTF2_VERSION_BUGFIX_DEF}")
string(REGEX MATCH "[0-9]+" OTF2_VERSION_BUGFIX "${OTF2_VERSION_BUGFIX_R}")

message(STATUS "Using OTF2 v${OTF2_VERSION_MAJOR}.${OTF2_VERSION_MINOR}.${OTF2_VERSION_BUGFIX}")

if(NOT "${OTF2_VERSION_MAJOR}.${OTF2_VERSION_MINOR}" STREQUAL "2.3")
    message(FATAL_ERROR "Otter requires OTF2 v2.3")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # Build examples for demonstrating Otter  # # # # # # # # # # # # # # #

message(STATUS "Detecting examples")
if (WITH_EXAMPLES)
    message(STATUS "Adding examples")
    add_subdirectory(src/otter/examples)
endif()

# # # Include testing # # # # # # # # # # # # # # # # # # # # # # # # # # #

if (WITH_TESTS)
    add_subdirectory(test)
endif()
