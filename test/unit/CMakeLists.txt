
# # # GoogleTest Framework # # # # # # # # # # # # # # # # # # # # # # # # #

message(STATUS "Unit tests enabled")

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/GoogleTest.git
  GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723 # 1.17.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_MakeAvailable(googletest)
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
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

add_executable(
    vptr_manager_test
    vptr_manager_test.cpp
)
target_include_directories(
    vptr_manager_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries(
    vptr_manager_test
    gtest_main
    $<TARGET_OBJECTS:otter-dtype>
)

include(GoogleTest)
gtest_discover_tests(queue_test)
gtest_discover_tests(stack_test)
gtest_discover_tests(string_registry_test)
gtest_discover_tests(vptr_manager_test)
