
# # # GoogleTest Framework # # # # # # # # # # # # # # # # # # # # # # # # #

message(STATUS "Tests enabled")

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/GoogleTest.git
  GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929
)

# FetchContent_MakeAvailable(googletest)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(googletest)

  # Bring the populated content into the build
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

enable_testing()

add_executable(
    queue_test
    queue_test.cc
)
target_include_directories(
    queue_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    queue_test
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

add_executable(
    stack_test
    stack_test.cc
)
target_include_directories(
    stack_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    stack_test
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

add_executable(
    char_ref_registry_test
    char_ref_registry_test.cc
)
target_include_directories(
    char_ref_registry_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    char_ref_registry_test
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

add_executable(
    value_registry_test
    value_registry_test.cpp
)
target_include_directories(
    value_registry_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    value_registry_test
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

add_library(string_registry_lib OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/otter/types/string_value_registry.cpp"
)
target_include_directories(
    string_registry_lib
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
add_executable(
    string_registry_test
    string_value_registry_test.cpp
)
target_include_directories(
    string_registry_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    string_registry_test
    string_registry_lib
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

include(GoogleTest)
gtest_discover_tests(queue_test)
gtest_discover_tests(stack_test)
gtest_discover_tests(char_ref_registry_test)
gtest_discover_tests(value_registry_test)
gtest_discover_tests(string_registry_test)
