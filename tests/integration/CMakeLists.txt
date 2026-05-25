cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-itegtest VERSION ${version})

file(GLOB test_gtest_src "${TEST_ROOT}/external/googletest/src/gtest-all.cc")

file(GLOB test_src "*.cpp")
file(GLOB_RECURSE test_sync_src "sync/*.cpp")
file(GLOB_RECURSE test_async_src "async/*.cpp")

add_executable(${PROJECT_NAME}
	${test_gtest_src}
	${test_src}
	${test_sync_src}
	${test_async_src}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${TEST_ROOT}/external/httplib
	PRIVATE ${TEST_ROOT}/external/googletest/include
	PRIVATE ${TEST_ROOT}/external/googletest/)

target_link_libraries(${PROJECT_NAME} cpp-sdk-v2)
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
    PRIVATE "${SDK_COMPILER_FLAGS}"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_NO_EXCEPTIONS)

add_test(NAME cpp-sdk-itegtest COMMAND ${PROJECT_NAME})

