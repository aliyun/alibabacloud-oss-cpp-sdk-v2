
project(cpp-sdk-v2 VERSION ${version})

if(USE_CURL_TRANSPORT)
    set(ALIBABACLOUD_OSS_HAS_CURL 1)
endif()
if(USE_WINHTTP_TRANSPORT AND WIN32)
    set(ALIBABACLOUD_OSS_HAS_WINHTTP 1)
endif()

set(ALIBABACLOUD_OSS_USE_STD_EXPECTED ${USE_STD_EXPECTED})
configure_file(src/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/alibabacloud/oss2/Config.h)

#common header
file(GLOB sdk_base_header "include/alibabacloud/oss2/*.h")
file(GLOB sdk_credentials_header "include/alibabacloud/oss2/credentials/*.h")
file(GLOB sdk_signer_header "include/alibabacloud/oss2/signer/*.h")
file(GLOB sdk_retry_header "include/alibabacloud/oss2/retry/*.h")
file(GLOB sdk_transport_header "include/alibabacloud/oss2/transport/*.h")
file(GLOB sdk_utils_header "include/alibabacloud/oss2/utils/*.h")
file(GLOB sdk_io_header "include/alibabacloud/oss2/io/*.h")

#all header
file(GLOB sdk_header
    ${sdk_base_header}
    ${sdk_credentials_header}
    ${sdk_signer_header}
    ${sdk_retry_header}
    ${sdk_transport_header}
    ${sdk_utils_header}
    ${sdk_io_header}
)

#common source
file(GLOB sdk_base_src "src/*.cpp")
file(GLOB sdk_credentials_src "src/credentials/*.cpp")
file(GLOB sdk_signer_src "src/signer/*.cpp")
file(GLOB sdk_retry_src "src/retry/*.cpp")
file(GLOB sdk_utils_src "src/utils/*.cpp")
file(GLOB sdk_client_sync_src "src/client/sync/*.cpp")
file(GLOB sdk_client_async_src "src/client/async/*.cpp")
file(GLOB sdk_internal_src "src/internal/*.cpp")
file(GLOB sdk_internal_sync_src "src/internal/sync/*.cpp")
file(GLOB sdk_internal_async_src "src/internal/async/*.cpp")
file(GLOB sdk_error_src "src/error/*.cpp")
file(GLOB sdk_io_src "src/io/*.cpp")
file(GLOB sdk_models_src "src/models/*.cpp")
file(GLOB sdk_transform_src "src/transform/*.cpp")

# transport
file(GLOB sdk_transport_common_src "src/transport/*.cpp")
set(sdk_transport_src ${sdk_transport_common_src})

if(USE_CURL_TRANSPORT)
    file(GLOB sdk_transport_curl_src "src/transport/curl/*.cpp")
    list(APPEND sdk_transport_src ${sdk_transport_curl_src})
endif()

if(USE_WINHTTP_TRANSPORT AND WIN32)
    file(GLOB sdk_transport_winhttp_src "src/transport/winhttp/*.cpp")
    list(APPEND sdk_transport_src ${sdk_transport_winhttp_src})
endif()

# add source by DISABLE_XX option

# external source
file(GLOB sdk_thirdparty_src
    "src/thirdparty/base64/*.cpp"
    "src/thirdparty/tinyxml2/*.cpp"
)

if(USE_SYSTEM_OPENSSL)
    set(sdk_utils_hash_src
        "src/utils/hash/Md5UtilsOpenssl.cpp"
        "src/utils/hash/HashUtilsOpenssl.cpp"
    )
elseif(USE_SYSTEM_MBEDTLS)
    set(sdk_utils_hash_src
        "src/utils/hash/Md5UtilsMbedtls.cpp"
        "src/utils/hash/HashUtilsMbedtls.cpp"
    )
else()
    set(sdk_utils_hash_src
        "src/utils/hash/Md5Utils.cpp"
        "src/utils/hash/HashUtils.cpp"
    )
    file(GLOB sdk_thirdparty_hash_src "src/thirdparty/hash/*.cpp")
    list(APPEND sdk_thirdparty_src ${sdk_thirdparty_hash_src})
endif()

#all source
file(GLOB sdk_src
    ${sdk_base_src}
    ${sdk_credentials_src}
    ${sdk_signer_src}
    ${sdk_retry_src}
    ${sdk_transport_src}
    ${sdk_error_src}
    ${sdk_utils_src}
    ${sdk_utils_hash_src}
    ${sdk_client_sync_src}
    ${sdk_client_async_src}
    ${sdk_internal_src}
    ${sdk_internal_sync_src}
    ${sdk_internal_async_src}
    ${sdk_io_src}
    ${sdk_models_src}
    ${sdk_transform_src}
    ${sdk_thirdparty_src}
)

if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED
        ${sdk_header}
        ${sdk_src})
else()
    add_library(${PROJECT_NAME} STATIC
        ${sdk_header}
        ${sdk_src})
endif()

add_library(alibabacloud_oss_v2::oss ALIAS ${PROJECT_NAME})
    

if(USE_CURL_TRANSPORT)
    if(USE_SYSTEM_CURL)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIB})
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CURL_LIB}>)
    endif()
endif()

if(USE_WINHTTP_TRANSPORT AND WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winhttp)
endif()

if(USE_SYSTEM_OPENSSL)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)
elseif(USE_SYSTEM_MBEDTLS)
    target_link_libraries(${PROJECT_NAME} PRIVATE MbedTLS::mbedcrypto)
endif()


set_target_properties(${PROJECT_NAME}
    PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}${PROJECT_NAME}
    EXPORT_NAME oss
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alibabacloud/oss2
        ${CMAKE_CURRENT_SOURCE_DIR}/src/external/
)

if(USE_STD_EXPECTED)
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
else()
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
endif()

#warning flags
if(MSVC)
    list(APPEND SDK_WARNING_FLAGS "/W4" "/D_CRT_SECURE_NO_WARNINGS" "/we4819")
else()
    list(APPEND SDK_WARNING_FLAGS "-Wall" "-Werror" "-pedantic" "-Wextra")
endif()

target_compile_options(${PROJECT_NAME} 
    PRIVATE "${SDK_COMPILER_FLAGS}"
    PRIVATE "${SDK_WARNING_FLAGS}"
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ALIBABACLOUD_OSS_SHARED ALIBABACLOUD_OSS_EXPORTS)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/alibabacloud_oss_v2)

install(TARGETS ${PROJECT_NAME}
    EXPORT alibabacloud_oss_v2Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT alibabacloud_oss_v2Targets
    NAMESPACE alibabacloud_oss_v2::
    DESTINATION ${INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/alibabacloud_oss_v2ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/alibabacloud_oss_v2Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/alibabacloud_oss_v2Config.cmake"
    INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/alibabacloud_oss_v2Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/alibabacloud_oss_v2ConfigVersion.cmake"
    DESTINATION ${INSTALL_CMAKEDIR}
)

install(DIRECTORY "include/alibabacloud" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

