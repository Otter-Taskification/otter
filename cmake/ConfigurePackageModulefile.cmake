#[=============================================================================[
Provides wrappers around configure_file which helps generate a modulefile for
installation with a package

Usage:

configure_package_modulefile(<input> <output>
    INSTALL_DESTINATION <path>
    [PATH_VARS <var1> <varN>]
)

Input Variables:

``<input>``
  The file to configure
``<output>``
  The output configured file
``INSTALL_DESTINATION <path>``
  The relative path in the install tree where the final modulefile will be
  installed
``PATH_VARS <var1> .. <varN>``
  Path variables to insert into the modulefile (not currently used)
#]=============================================================================]

function(configure_package_modulefile input output)
    if(${ARGC} LESS 4)
        message(FATAL_ERROR "Insufficient arguments")
    endif()

    if(NOT "${ARGV2}" STREQUAL "INSTALL_DESTINATION")
        message(FATAL_ERROR "Incorrect positional arguments <input>/<output>")
    endif()

    set(named_options "")
    set(one_value_keywords INSTALL_DESTINATION)
    set(multi_value_keywords PATH_VARS)
    cmake_parse_arguments(PARSE_ARGV 0 MODULEFILE "${named_options}" "${one_value_keywords}"
        "${multi_value_keywords}")

    if(NOT MODULEFILE_INSTALL_DESTINATION)
        message(FATAL_ERROR "INSTALL_DESTINATION is a required argument")
    endif()

    set(_dir "${MODULEFILE_INSTALL_DESTINATION}")
    set(MODULE_ROOT_PREFIX "")

    while(NOT ${_dir} STREQUAL ".")
        cmake_path(APPEND _dir "..")
        cmake_path(NORMAL_PATH _dir)
        string(APPEND MODULE_ROOT_PREFIX "../")
    endwhile()

    # MODULE_ROOT_PREFIX now holds the relative path to the installation root
    # for the given installation prefix of the modulefile in the installation
    # tree
    configure_file(${input} ${output} @ONLY)
endfunction()
