cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-itegtest VERSION ${version})

file(GLOB test_gtest_src "${TEST_ROOT}/external/googletest/src/gtest-all.cc")

file(GLOB test_src "*.cpp")
file(GLOB test_bucket_src "bucket/*.cpp")
file(GLOB test_object_src "object/*.cpp")


add_executable(${PROJECT_NAME}
	${test_gtest_src}
	${test_src}
	${test_bucket_src}
	${test_object_src}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
	PRIVATE ${TEST_ROOT}/external/googletest/include
    PRIVATE ${TEST_ROOT}/external/googletest/)

target_link_libraries(${PROJECT_NAME} cpp-sdk-v2 ${CURL_LIB})

target_compile_options(${PROJECT_NAME}
    PRIVATE "${SDK_COMPILER_FLAGS}"
)

if(CMAKE_HOST_UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)	
endif()

