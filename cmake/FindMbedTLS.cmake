# FindMbedTLS.cmake
# Locate mbedTLS library
#
# Defines:
#   MbedTLS_FOUND
#   MBEDTLS_INCLUDE_DIRS
#   MBEDCRYPTO_LIBRARY
#   MbedTLS::mbedcrypto  - imported target

find_path(MBEDTLS_INCLUDE_DIRS mbedtls/md.h)
find_library(MBEDCRYPTO_LIBRARY NAMES mbedcrypto)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MbedTLS
    REQUIRED_VARS MBEDCRYPTO_LIBRARY MBEDTLS_INCLUDE_DIRS
)

if(MbedTLS_FOUND AND NOT TARGET MbedTLS::mbedcrypto)
    add_library(MbedTLS::mbedcrypto UNKNOWN IMPORTED)
    set_target_properties(MbedTLS::mbedcrypto PROPERTIES
        IMPORTED_LOCATION "${MBEDCRYPTO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MBEDTLS_INCLUDE_DIRS}"
    )
endif()
