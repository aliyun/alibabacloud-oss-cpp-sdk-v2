vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO aliyun/alibabacloud-oss-cpp-sdk-v2
    REF "${VERSION}"
    SHA512 0
    HEAD_REF main
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        curl        USE_CURL_TRANSPORT
        curl        USE_SYSTEM_CURL
        winhttp     USE_WINHTTP_TRANSPORT
        openssl     USE_SYSTEM_OPENSSL
        mbedtls     USE_SYSTEM_MBEDTLS
        encryption  ENABLE_ENCRYPTION
        rtti        ENABLE_RTTI
)

if("openssl" IN_LIST FEATURES AND "mbedtls" IN_LIST FEATURES)
    message(WARNING "openssl and mbedtls are mutually exclusive; using openssl")
    list(APPEND FEATURE_OPTIONS "-DUSE_SYSTEM_MBEDTLS=OFF")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    DISABLE_PARALLEL_CONFIGURE
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_SAMPLES=OFF
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME alibabacloud_oss_v2 CONFIG_PATH lib/cmake/alibabacloud_oss_v2)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
