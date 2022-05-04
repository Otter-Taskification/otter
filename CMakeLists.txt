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
set(OTF2_INSTALL_DIR "/opt/otf2" CACHE STRING "OTF2 install directory")

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
elseif(CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM")
    if("${C_COMPILER_VERSION}" VERSION_GREATER_EQUAL "13.0.0")
        message(STATUS "Detected IntelLLVM >= 13.0.0")
	set(USE_OMPT_51 1)
    endif()
else()
    message(WARN " Selected C compiler may not be supported:")
    message(WARN "   CMAKE_C_COMPILER_ID=${CMAKE_C_COMPILER_ID}")
    message(WARN "   C_COMPILER_VERSION=${C_COMPILER_VERSION}")
endif()

# OMPT event source
add_library(otter-events-ompt OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/events/ompt/otter-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/events/ompt/otter-entry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/types/otter-structs.c
)
target_include_directories(otter-events-ompt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(otter-events-ompt BEFORE PRIVATE ${CMAKE_C_FLAGS})
target_compile_definitions(otter-events-ompt PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_CORE})
set_property(TARGET otter-events-ompt PROPERTY POSITION_INDEPENDENT_CODE ON)

# Serial event source
add_library(otter-events-serial OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/events/serial/otter-serial.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/types/otter-structs.c
)
target_include_directories(otter-events-serial PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(otter-events-serial BEFORE PRIVATE ${CMAKE_C_FLAGS})
target_compile_definitions(otter-events-serial PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_CORE})
set_property(TARGET otter-events-serial PROPERTY POSITION_INDEPENDENT_CODE ON)

# Trace component for OMP mode
add_library(otter-trace-ompt OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-location.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-parallel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-workshare.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-archive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-unique-refs.c
)
target_include_directories(otter-trace-ompt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(otter-trace-ompt PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_TRACE})
set_property(TARGET otter-trace-ompt PROPERTY POSITION_INDEPENDENT_CODE ON)

# Trace component for serial mode
add_library(otter-trace-serial OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-location.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-parallel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-workshare.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-region-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-archive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/trace/trace-unique-refs.c
)
target_include_directories(otter-trace-serial PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(otter-trace-serial PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_TRACE})
set_property(TARGET otter-trace-serial PROPERTY POSITION_INDEPENDENT_CODE ON)
# Add define to signify serial tracing mode
target_compile_definitions(otter-trace-serial PRIVATE OTTER_SERIAL_MODE)

# Datatypes component
add_library(otter-dtype OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/types/dt-queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/types/dt-stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/otter/types/char_ref_registry.cpp
)
target_include_directories(otter-dtype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(otter-dtype PRIVATE DEBUG_LEVEL=${OTTER_DEBUG_DTYPE})
set_property(TARGET otter-dtype PROPERTY POSITION_INDEPENDENT_CODE ON)

# Otter library compiled for OMP mode
add_library(otter-ompt
    $<TARGET_OBJECTS:otter-events-ompt>
    $<TARGET_OBJECTS:otter-trace-ompt>
    $<TARGET_OBJECTS:otter-dtype>
)

# Otter library compiled for serial mode
add_library(otter-serial
    $<TARGET_OBJECTS:otter-events-serial>
    $<TARGET_OBJECTS:otter-trace-serial>
    $<TARGET_OBJECTS:otter-dtype>
)
set_target_properties(otter-serial PROPERTIES PUBLIC_HEADER "include/otter/otter-serial.h")

# Specify Otter install location
install(TARGETS otter-ompt DESTINATION lib)
install(TARGETS otter-serial LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/otter)

# Configure version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otter/otter-version.h
)

# Link to dependencies
target_link_libraries(otter-ompt pthread)
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
    target_compile_definitions(otter-events-ompt PRIVATE USE_OMPT_MASKED)
    target_compile_definitions(otter-trace-ompt PRIVATE USE_OMPT_MASKED)
else()
    message(STATUS "Not using OMPT 5.1")
endif()

# # # OTF2 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add OTF2 install directory if it was specified
set(HAVE_OTF2_INSTALL_DIR FALSE)
if(DEFINED OTF2_INSTALL_DIR)
    target_include_directories(otter-events-ompt PRIVATE ${OTF2_INSTALL_DIR}/include)
    target_include_directories(otter-events-serial PRIVATE ${OTF2_INSTALL_DIR}/include)
    target_include_directories(otter-trace-ompt PRIVATE ${OTF2_INSTALL_DIR}/include)
    target_include_directories(otter-trace-serial PRIVATE ${OTF2_INSTALL_DIR}/include)
    target_include_directories(otter-dtype PRIVATE ${OTF2_INSTALL_DIR}/include)
    set(HAVE_OTF2_INSTALL_DIR TRUE)
    message(STATUS "OTF2 include dir was specified")
endif()

target_link_libraries(otter-ompt otf2)
target_link_libraries(otter-serial otf2)
# Add library directory for OTF2 if it was specified
if(DEFINED OTF2_INSTALL_DIR)
    target_link_libraries(otter-ompt -L${OTF2_INSTALL_DIR}/lib)
    target_link_libraries(otter-serial -L${OTF2_INSTALL_DIR}/lib)
endif()

# Check for otf2.h
set(CMAKE_REQUIRED_INCLUDES $<$<BOOL:HAVE_OTF2_INSTALL_DIR>:${OTF2_INSTALL_DIR}/include>)
CHECK_INCLUDE_FILE(otf2/otf2.h HAVE_OTF2_H)
if(NOT HAVE_OTF2_H)
    message(FATAL_ERROR "Can't find otf2.h - did you specify OTF2_INSTALL_DIR?")
endif()

# Check OTF2 version found
file(READ "${OTF2_INSTALL_DIR}/include/otf2/OTF2_GeneralDefinitions.h" otf2_defs_h)

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
