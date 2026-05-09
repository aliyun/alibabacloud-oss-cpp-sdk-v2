
project(cpp-sdk-v2 VERSION ${version})

configure_file(src/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/alibabacloud/oss2/Config.h @ONLY)

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
)

#common source
file(GLOB sdk_base_src "src/*.cpp")
file(GLOB sdk_credentials_src "src/credentials/*.cpp")
file(GLOB sdk_signer_src "src/signer/*.cpp")
file(GLOB sdk_retry_src "src/retry/*.cpp")
file(GLOB sdk_utils_src "src/utils/*.cpp")
file(GLOB sdk_internal_src "src/internal/*.cpp")
file(GLOB sdk_io_src "src/io/*.cpp")
file(GLOB sdk_models_src "src/models/*.cpp")
file(GLOB sdk_transform_src "src/transform/*.cpp")

# transport 
file(GLOB sdk_transport_common_src "src/transport/*.cpp")
file(GLOB sdk_transport_curl_src "src/transport/curl/*.cpp")
file(GLOB sdk_transport_src
    ${sdk_transport_common_src}
    ${sdk_transport_curl_src}
)

# add source by DISABLE_XX option

# external source
file(GLOB sdk_thirdparty_base64_src "src/thirdparty/base64/*.cpp")
file(GLOB sdk_thirdparty_tinyxml2_src "src/thirdparty/tinyxml2/*.cpp")
file(GLOB sdk_thirdparty_hash_src "src/thirdparty/hash/*.cpp")

file(GLOB sdk_thirdparty_src
    ${sdk_thirdparty_base64_src}
    ${sdk_thirdparty_tinyxml2_src}
    ${sdk_thirdparty_hash_src}
)

#all source
file(GLOB sdk_src
    ${sdk_base_src}
    ${sdk_credentials_src}
    ${sdk_signer_src}
    ${sdk_retry_src}
    ${sdk_transport_src}
    ${sdk_utils_src}
    ${sdk_internal_src}
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
    

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${CURL_LIB}
)

# Fix missing OpenSSL includes for Windows since in 'ssl_ctx.cpp' we include OpenSSL directly
if(SSL_BACKEND_USED STREQUAL "OpenSSL")
    target_link_libraries(cpp-sdk PRIVATE OpenSSL::SSL)
    target_include_directories(cpp-sdk PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}${PROJECT_NAME}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE include
    PRIVATE include/alibabacloud/oss2    
    PRIVATE src/external/
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

#warning flags
if(MSVC)
    list(APPEND SDK_WARNING_FLAGS "/W4" "/D_CRT_SECURE_NO_WARNINGS")
else()
    list(APPEND SDK_WARNING_FLAGS "-Wall" "-Werror" "-pedantic" "-Wextra")
endif()

target_compile_options(${PROJECT_NAME} 
    PRIVATE "${SDK_COMPILER_FLAGS}"
    PRIVATE "${SDK_WARNING_FLAGS}"
)
if(BUILD_SHARED_LIBS)
    target_compile_options(${PROJECT_NAME}
        PRIVATE -DALIBABACLOUD_OSS_SHARED -DALIBABACLOUD_OSS_EXPORTS)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY "include/alibabacloud" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})