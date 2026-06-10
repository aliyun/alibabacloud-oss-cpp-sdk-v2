cmake_minimum_required(VERSION 3.15)

#Project setting
file(STRINGS "VERSION" version)
project(alibabacloud-oss-cpp-sdk VERSION  ${version} LANGUAGES CXX)
set(TARGET_OUTPUT_NAME_PREFIX "alibabacloud-oss-" CACHE STRING "The target's output name prefix")

message(STATUS "Project version: ${PROJECT_VERSION}")

# C++ Standard
if (CMAKE_PARENT_CXX_STANDARD)
    if (CMAKE_PARENT_CXX_STANDARD LESS 17)
        message(FATAL_ERROR "oss sdk does not support ${CMAKE_PARENT_CXX_STANDARD}")
    endif()
elseif (CMAKE_CXX_STANDARD)
    if (CMAKE_CXX_STANDARD LESS 17)
        message(FATAL_ERROR "oss sdk does not support ${CMAKE_CXX_STANDARD}")
    endif()
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option
macro(sdk_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()
message(STATUS "C++ CMake Options")
message(STATUS "=======================================================")
sdk_option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
sdk_option(BUILD_TESTS "Build unit tests" OFF)
sdk_option(BUILD_SAMPLES "Build samples" OFF)

sdk_option(ENABLE_RTTI "Flag to enable/disable building code with RTTI information" OFF)

sdk_option(USE_SYSTEM_CURL "If enabled we will use the curl lib already installed on this system." OFF)
sdk_option(USE_SYSTEM_OPENSSL "If enabled we will use the openssl lib already installed on this system." OFF)
sdk_option(USE_SYSTEM_MBEDTLS "If enabled we will use the mbedtls lib already installed on this system." OFF)
sdk_option(USE_SYSTEM_TINYXML2 "If enabled we will use the tinyxml2 lib already installed on this system." OFF)

sdk_option(USE_CURL_TRANSPORT "Compile curl transport support" ON)
sdk_option(USE_WINHTTP_TRANSPORT "Compile WinHTTP transport support (Windows only)" OFF)

sdk_option(USE_STD_EXPECTED "Use std::expected instead of Outcome (requires C++23)" OFF)

sdk_option(ENABLE_ENCRYPTION "Enable client-side encryption" OFF)

sdk_option(ENABLE_COVERAGE "Set to ON to generate coverage reports." OFF)
sdk_option(ENABLE_CPPCHECK "Set to ON to enable Cppcheck static analysis." OFF)
sdk_option(ENABLE_SANITIZER "Set to ON to enable sanitizers." OFF)
message(STATUS "=======================================================")

option(BUILD_PERF_TESTS "Build performance tests" OFF)

#Platform

# Find Dependencies
#find_package(CURL REQUIRED)
#find_package(OpenSSL REQUIRED)
#find_package(Threads REQUIRED)

#Compiler flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
message(STATUS "CXX Standard: ${CMAKE_CXX_STANDARD}")
if(MSVC)
    list(APPEND SDK_COMPILER_FLAGS "/MP")
    if (NOT ENABLE_RTTI)
    list(APPEND SDK_COMPILER_FLAGS "/GR-")
    endif()
else()
    list(APPEND SDK_COMPILER_FLAGS "-fno-exceptions")
    if (NOT ENABLE_RTTI)
    list(APPEND SDK_COMPILER_FLAGS "-fno-rtti")
    endif()
endif()

include(ExternalProject)
include(GNUInstallDirs)
include(FetchContent)

# CMake can find FindMbedTLS.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# code coverage
if(ENABLE_COVERAGE)
    include(cmake/code_coverage.cmake)
endif()

# Cppcheck
if(ENABLE_CPPCHECK)
    include(cmake/cppcheck.cmake)
endif()

# SANITIZER
if(ENABLE_SANITIZER)
    include(cmake/sanitizer.cmake)
endif()

#Find dependency Library for hash
if(USE_SYSTEM_OPENSSL AND USE_SYSTEM_MBEDTLS)
    message(FATAL_ERROR "USE_SYSTEM_OPENSSL and USE_SYSTEM_MBEDTLS are mutually exclusive. Please enable only one.")
endif()

if(USE_SYSTEM_OPENSSL)
    find_package(OpenSSL REQUIRED)
    message(STATUS "Using system OpenSSL for hash: ${OPENSSL_VERSION}")
endif()

if(USE_SYSTEM_MBEDTLS)
    find_package(MbedTLS CONFIG QUIET)
    if(NOT MbedTLS_FOUND)
        find_package(MbedTLS REQUIRED)
    endif()
    message(STATUS "Using system mbedTLS for hash")
endif()

#Find dependency Library tinyxml2
if(USE_SYSTEM_TINYXML2)
    find_package(tinyxml2 CONFIG REQUIRED)
    message(STATUS "Using system tinyxml2")
endif()

#Find dependency Library curl
if(USE_CURL_TRANSPORT)
    if(USE_SYSTEM_CURL)
        find_package(CURL REQUIRED)
    else()
        message(STATUS "Configuring built-in curl...")
        # --- CURL ---
        FetchContent_Declare(
            curl
            URL https://github.com/curl/curl/releases/download/curl-8_13_0/curl-8.13.0.tar.xz
            URL_HASH SHA256=4a093979a3c2d02de2fbc00549a32771007f2e78032c6faa5ecd2f7a9e152025
        )

        # Common curl options
        set(BUILD_CURL_EXE OFF CACHE BOOL "Disable curl CLI" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "Build curl examples" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "Disable curl tests" FORCE)
        set(CURL_DISABLE_TESTS ON CACHE BOOL "Disable curl-specific tests" FORCE)
        set(CURL_DISABLE_INSTALL ON CACHE BOOL "Disable curl install" FORCE)

        # Disable unnecessary protocols
        set(HTTP_ONLY ON CACHE BOOL "Only need HTTP (and HTTPS) support" FORCE)
        set(CURL_USE_LIBPSL OFF CACHE INTERNAL "Disable lib psl" FORCE)
        # Platform-specific TLS backend
        if(WIN32)
            set(CURL_USE_SCHANNEL ON CACHE BOOL "Use Windows native SSL backend" FORCE)
        elseif(ANDROID)
            message(STATUS "Disabling TLS for Android cross-compilation")
            set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
            set(CURL_USE_MBEDTLS OFF CACHE BOOL "" FORCE)
            set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
            set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
        else()
            find_package(OpenSSL QUIET)
            if(OpenSSL_FOUND)
                message(STATUS "Using OpenSSL for TLS")
                set(OPENSSL_USE_STATIC_LIBS TRUE)
                set(CMAKE_USE_OPENSSL ON CACHE BOOL "" FORCE)
                set(USE_OPENSSL ON CACHE BOOL "" FORCE)
            else()
                find_package(GnuTLS QUIET)
                if(GnuTLS_FOUND)
                    message(STATUS "Using GnuTLS for TLS")
                    set(GNUTLS_USE_STATIC_LIBS TRUE)
                    set(CMAKE_USE_GNUTLS ON CACHE BOOL "" FORCE)
                    set(USE_GNUTLS ON CACHE BOOL "" FORCE)
                else()
                    message(FATAL_ERROR "No supported TLS backend (OpenSSL or GnuTLS) found!")
                endif()
            endif()
        endif()
        FetchContent_GetProperties(curl)
        if(NOT curl_POPULATED)
            FetchContent_Populate(curl)
            add_subdirectory(${curl_SOURCE_DIR} ${curl_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()
    endif()

    if(TARGET libcurl)
        set(CURL_LIB libcurl)
    else()
        set(CURL_LIB CURL::libcurl)
    endif()
endif()

add_subdirectory(sdk)

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

