cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-perftest VERSION ${version})

include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

file(GLOB perf_src "*.cpp")

add_executable(${PROJECT_NAME} ${perf_src})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
    PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    cpp-sdk-v2
    ${CURL_LIB}
    benchmark::benchmark
)

target_compile_options(${PROJECT_NAME}
    PRIVATE "${SDK_COMPILER_FLAGS}")

if(CMAKE_HOST_UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()
