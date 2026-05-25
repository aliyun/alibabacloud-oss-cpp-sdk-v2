cmake_minimum_required(VERSION 3.15)

set(TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(e2e-mock)

if(BUILD_PERF_TESTS)
    add_subdirectory(perf)
endif()
