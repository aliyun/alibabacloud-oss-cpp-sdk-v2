cmake_minimum_required(VERSION 3.15)

project(cpp-sdk-unittest VERSION ${version})

file(GLOB test_gtest_src "${TEST_ROOT}/external/googletest/src/gtest-all.cc")

file(GLOB test_unit_src "*.cpp")
file(GLOB_RECURSE test_unit_client_src "client/*.cpp")
file(GLOB_RECURSE test_unit_credentail_src "credentials/*.cpp")
file(GLOB_RECURSE test_unit_retry_src "retry/*.cpp")
file(GLOB_RECURSE test_unit_internal_src "internal/*.cpp")
file(GLOB_RECURSE test_unit_models_src "models/*.cpp")
file(GLOB_RECURSE test_unit_utils_src "utils/*.cpp")
file(GLOB_RECURSE test_unit_error_src "error/*.cpp")
file(GLOB_RECURSE test_unit_io_src "io/*.cpp")
file(GLOB_RECURSE test_unit_signer_src "signer/*.cpp")
file(GLOB_RECURSE test_unit_transform_src "transform/*.cpp")
file(GLOB test_unit_transport_src "transport/*.cpp")
if(USE_CURL_TRANSPORT)
    file(GLOB_RECURSE test_unit_transport_curl_src "transport/curl/*.cpp")
    list(APPEND test_unit_transport_src ${test_unit_transport_curl_src})
endif()
if(USE_WINHTTP_TRANSPORT AND WIN32)
    file(GLOB_RECURSE test_unit_transport_winhttp_src "transport/winhttp/*.cpp")
    list(APPEND test_unit_transport_src ${test_unit_transport_winhttp_src})
endif()
if(ENABLE_ENCRYPTION)
    file(GLOB_RECURSE test_unit_encryption_src "crypto/*.cpp")
else()
    set(test_unit_encryption_src "")
endif()

add_executable(${PROJECT_NAME}
	${test_gtest_src}
	${test_unit_src}
	${test_unit_client_src}
	${test_unit_credentail_src}
	${test_unit_retry_src}
	${test_unit_internal_src}
	${test_unit_models_src}
	${test_unit_utils_src}
	${test_unit_io_src}
	${test_unit_signer_src}
	${test_unit_transform_src}
	${test_unit_error_src}
	${test_unit_transport_src}
	${test_unit_encryption_src}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${TEST_ROOT}/external/googletest/include
	PRIVATE ${TEST_ROOT}/external/googletest/)

target_link_libraries(${PROJECT_NAME} cpp-sdk-v2)
if(USE_CURL_TRANSPORT)
    target_link_libraries(${PROJECT_NAME} ${CURL_LIB})
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE "${SDK_COMPILER_FLAGS}")


add_test(NAME cpp-sdk-unittest COMMAND ${PROJECT_NAME})
