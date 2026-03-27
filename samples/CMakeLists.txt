cmake_minimum_required(VERSION 3.15)

project(oss-cpp-sdk-v2-samples)

file(GLOB SAMPLE_SRCS "*.cpp")

foreach(SAMPLE_SRC ${SAMPLE_SRCS})
    get_filename_component(SAMPLE_NAME ${SAMPLE_SRC} NAME_WE)
    add_executable(sample_${SAMPLE_NAME} ${SAMPLE_SRC})
    target_include_directories(sample_${SAMPLE_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
    target_link_libraries(sample_${SAMPLE_NAME} PRIVATE cpp-sdk-v2)
    target_compile_options(sample_${SAMPLE_NAME}
        PRIVATE "${SDK_COMPILER_FLAGS}")
endforeach()
