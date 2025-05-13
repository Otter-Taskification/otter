#[=======================================================================[.rst:
FindOTF2
--------

Find the OTF2 library.

Usage
^^^^^

``find_package(OTF2)``

This script serves two purposes:

1. Allow Otter to discover an OTF2 installation when building Otter.
2. Allow an Otter installation to discover its OTF2 dependency on behalf of a
    package which uses Otter via `find_package(Otter)`. This script is installed
    with Otter and included by OtterConfig.cmake. Consumers of Otter can still
    modify this process by setting OTF2_INSTALL_DIR but this is not required.

Input Variables
^^^^^^^^^^^^^^^

Modify this module with the following variables:

``OTF2_INSTALL_DIR``
  The root of your preferred OTF2 installation.

Result Variables
^^^^^^^^^^^^^^^^

Sets the following variables:

``OTF2_FOUND``
  True if OTF2 was found, false otherwise.
``OTF2_ROOT``
  The root of the found OTF2 installation.
``OTF2_VERSION``
  The full version string.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported target, if OTF2 was found:

``OTF2::otf2``
  The OTF2 library
#]=======================================================================]

include(FindPackageHandleStandardArgs)

if(NOT OTF2_FOUND)

    # #########################################################
    # set the default search location
    # #########################################################
    set(OTF2_DEFAULT_INSTALL_DIR "/opt/otf2")
    set(OTF2_CONFIG "otf2-config")
    set(OTF2_HEADER "otf2/otf2.h")
    set(OTF2_LIBS libotf2.a libotf2.so libotf2 otf2)

    # #########################################################
    # define the search path, preferring a user-specified installation if given
    # #########################################################
    if(NOT OTF2_INSTALL_DIR)
        set(OTF2_INSTALL_DIR ${OTF2_DEFAULT_INSTALL_DIR})
    endif()
    set(OTF2_SEARCH_PATH ${OTF2_INSTALL_DIR})

    message(VERBOSE "OTF2 search path:")
    foreach(SEARCH_PATH IN ITEMS ${OTF2_SEARCH_PATH})
        message(VERBOSE "  ${SEARCH_PATH}")
    endforeach()

    # #########################################################
    # find the include dir
    # #########################################################
    find_path(OTF2_INCLUDE_DIR "${OTF2_HEADER}"
        PATHS ${OTF2_SEARCH_PATH}
        PATH_SUFFIXES include
    )
    if(NOT OTF2_INCLUDE_DIR)
        message(FATAL_ERROR "failed to find ${OTF2_HEADER} in ${OTF2_SEARCH_PATH}")
    endif()

    # #########################################################
    # find otf2-config
    # #########################################################
    find_program(OTF2_CONFIG_EXE "${OTF2_CONFIG}"
        PATHS ${OTF2_SEARCH_PATH}
        PATH_SUFFIXES bin
    )
    if(NOT OTF2_CONFIG_EXE)
        message(FATAL_ERROR "failed to find ${OTF2_CONFIG} in ${OTF2_SEARCH_PATH}")
    endif()

    # #########################################################
    # find the library, preferring static over shared
    # #########################################################
    find_library(OTF2_LIBRARY NAMES ${OTF2_LIBS}
        PATHS ${OTF2_SEARCH_PATH}
        PATH_SUFFIXES lib
    )
    if(NOT OTF2_LIBRARY)
        message(FATAL_ERROR "failed to find ${OTF2_LIBS} in ${OTF2_SEARCH_PATH}")
    endif()

    # #########################################################
    # store the installation root
    # #########################################################
    cmake_path(REMOVE_FILENAME OTF2_INCLUDE_DIR OUTPUT_VARIABLE OTF2_ROOT)

    # #########################################################
    # get the version string
    # #########################################################
    execute_process(COMMAND ${OTF2_CONFIG_EXE} --version OUTPUT_VARIABLE OTF2_VERSION_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "otf2-config: version \(.*\)" "\\1" OTF2_VERSION "${OTF2_VERSION_OUTPUT}")
    if(NOT OTF2_VERSION)
        message(FATAL_ERROR "failed to parse OTF2 version from ${OTF2_CONFIG_EXE}")
    endif()

    message(VERBOSE "OTF2 environment:")
    message(VERBOSE "  include:         ${OTF2_INCLUDE_DIR}")
    message(VERBOSE "  lib:             ${OTF2_LIBRARY}")
    message(VERBOSE "  version output:  '${OTF2_VERSION_OUTPUT}'")

    find_package_handle_standard_args(OTF2
        REQUIRED_VARS OTF2_ROOT OTF2_INCLUDE_DIR OTF2_LIBRARY
        VERSION_VAR OTF2_VERSION
        NAME_MISMATCHED # prevent name mismatch warnings when an Otter installation finds OTF2
    )

    # #########################################################
    # create the target
    # #########################################################
    if(OTF2_FOUND AND NOT OTF2_VERSION VERSION_LESS 2.3 AND NOT TARGET OTF2::otf2)
        add_library(OTF2::otf2 IMPORTED INTERFACE)

        # add all the properties required to use OTF2
        set_target_properties(OTF2::otf2 PROPERTIES
            IMPORTED_LOCATION ${OTF2_LIBRARY}
            INTERFACE_LINK_LIBRARIES ${OTF2_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${OTF2_INCLUDE_DIR}
        )
    endif()

    unset(OTF2_DEFAULT_INSTALL_DIR)
    unset(OTF2_SEARCH_PATH)
    unset(OTF2_CONFIG)
    unset(OTF2_HEADER)
    unset(OTF2_LIBS)
endif()
