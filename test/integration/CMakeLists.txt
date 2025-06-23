message(STATUS "Integration test apps enabled")

find_package(OpenMP REQUIRED)

set(SCRIPT_PREFIX "${CMAKE_BINARY_DIR}/bin")
set(DATA_PREFIX "${CMAKE_BINARY_DIR}/data")

add_executable(
    taskwait
    taskwait.c
)

target_link_libraries(
    taskwait
    PRIVATE otter-task-graph
    OpenMP::OpenMP_C
)

set_target_properties(
    taskwait
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SCRIPT_PREFIX}/integration-tests"
)

message(STATUS "Generating run-all script for integration tests")
message(STATUS "SCRIPT_PREFIX: ${SCRIPT_PREFIX}")
message(STATUS "input:         ${CMAKE_SOURCE_DIR}/test/integration/run-all.sh.in")
message(STATUS "output:        ${SCRIPT_PREFIX}/generate-integration-tests-data.sh")

configure_file(
    "${CMAKE_SOURCE_DIR}/test/integration/run-all.in.sh"
    "${SCRIPT_PREFIX}/generate-integration-tests-data.sh"
    @ONLY
)

message(STATUS "generate test data with: '${SCRIPT_PREFIX}/generate-integration-tests-data.sh run'")
