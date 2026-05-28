vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO aliyun/alibabacloud-oss-cpp-sdk-v2
    REF "v${VERSION}"
    SHA512 0  # placeholder, update before registry submission
    HEAD_REF main
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        curl      USE_CURL_TRANSPORT
        winhttp   USE_WINHTTP_TRANSPORT
        openssl   USE_SYSTEM_OPENSSL
        mbedtls   USE_SYSTEM_MBEDTLS
        encryption ENABLE_ENCRYPTION
        rtti      ENABLE_RTTI
)

if("curl" IN_LIST FEATURES)
    list(APPEND FEATURE_OPTIONS -DUSE_SYSTEM_CURL=ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DBUILD_TESTS=OFF
        -DBUILD_SAMPLES=OFF
)

vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_cmake_config_fixup(PACKAGE_NAME alibabacloud_oss_v2 CONFIG_PATH lib/cmake/alibabacloud_oss_v2)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_copy_pdbs()
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
