# Build the OMPT plugin here, including all supporting logic
include(GNUInstallDirs)

# Provide the otter-ompt plugin
add_library(otter-ompt SHARED
    otter-core.c
    otter-entry.c
)

target_include_directories(otter-ompt
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> # for generated config.h and otter-version.h
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # for all other includes
)

target_link_libraries(otter-ompt
    PRIVATE otter-trace otter-dtype
    INTERFACE OTF2::otf2 # add to the link interface for consumers of otter-ompt
)

target_compile_definitions(otter-ompt
    PRIVATE
    DEBUG_LEVEL=$<IF:$<CONFIG:Debug>,3,0>
    DEPRECATION_WARNINGS
    USE_OMPT_MASKED
)

set_target_properties(otter-ompt PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# specifies rules for installing targets from a project
install(TARGETS otter-ompt)
