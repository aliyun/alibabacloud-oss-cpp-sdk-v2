cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-unittest VERSION ${version})

file(GLOB test_gtest_src "${TEST_ROOT}/external/googletest/src/gtest-all.cc")

file(GLOB test_unit_src "*.cpp")
file(GLOB test_unit_credentail_src "credentials/*.cpp")
file(GLOB test_unit_retry_src "retry/*.cpp")
file(GLOB test_unit_internal_src "internal/*.cpp")
file(GLOB test_unit_models_src "models/*.cpp")
file(GLOB test_unit_utils_src "utils/*.cpp")
file(GLOB test_unit_io_src "io/*.cpp")
file(GLOB test_unit_signer_src "signer/*.cpp")
file(GLOB test_unit_transform_src "transform/*.cpp")

add_executable(${PROJECT_NAME} 
	${test_gtest_src}
	${test_unit_src} 
	${test_unit_credentail_src} 
	${test_unit_retry_src} 
	${test_unit_internal_src} 
	${test_unit_models_src} 
	${test_unit_utils_src} 
	${test_unit_io_src}
	${test_unit_signer_src}
	${test_unit_transform_src}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
	PRIVATE ${TEST_ROOT}/external/googletest/include
    PRIVATE ${TEST_ROOT}/external/googletest/)

target_link_libraries(${PROJECT_NAME} cpp-sdk-v2)

target_compile_options(${PROJECT_NAME}
    PRIVATE "${SDK_COMPILER_FLAGS}")

if(CMAKE_HOST_UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)	
endif()
