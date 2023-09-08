include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Write the OTF2 installation we found to Otter's config file so that consumers
# of Otter get the same installation when they find_package(Otter). This can
# still be overridden if they set OTF2_INSTALL_DIR
set(OTF2_ROOT_GUESS ${OTF2_ROOT})

# generate the package config file that includes the exports
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/OtterConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/OtterConfig.cmake
    PATH_VARS OTF2_ROOT_GUESS
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/OtterConfigVersion.cmake
    VERSION "${Otter_VERSION_MAJOR}.${Otter_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/OtterConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/OtterConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Otter
)
