cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-perftest VERSION ${version})

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${TEST_ROOT}/external/benchmark ${CMAKE_CURRENT_BINARY_DIR}/benchmark)

file(GLOB perf_src "*.cpp")

add_executable(${PROJECT_NAME} ${perf_src})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    cpp-sdk-v2
    benchmark::benchmark
)
if(USE_CURL_TRANSPORT)
    target_link_libraries(${PROJECT_NAME} ${CURL_LIB})
endif()
if(USE_WINHTTP_TRANSPORT AND WIN32)
    target_link_libraries(${PROJECT_NAME} winhttp)
endif()
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE "${SDK_COMPILER_FLAGS}")

if(CMAKE_HOST_UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()
