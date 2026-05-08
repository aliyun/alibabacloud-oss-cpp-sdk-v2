cmake_minimum_required(VERSION 3.15)

set(TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

enable_testing()

add_subdirectory(unit)
add_subdirectory(integration)
