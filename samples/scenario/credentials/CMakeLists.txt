cmake_minimum_required(VERSION 3.15)
project(scenario-credentials)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

# Samples that only depend on the OSS SDK (built-in credential providers)
set(BUILTIN_SAMPLES
    StaticCredentials
    EnvironmentCredentials
    StsTokenCredentials
    CustomProviderFunc
)
foreach(SAMPLE ${BUILTIN_SAMPLES})
    add_executable(${SAMPLE} ${SAMPLE}.cpp)
    target_link_libraries(${SAMPLE} PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
endforeach()

# CredentialsCppIntegration requires alibabacloud-credentials-cpp
find_package(alibabacloud-credentials-cpp QUIET)
if(alibabacloud-credentials-cpp_FOUND)
    add_executable(CredentialsCppIntegration CredentialsCppIntegration.cpp)
    target_link_libraries(CredentialsCppIntegration
        PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2
        PRIVATE alibabacloud-credentials-cpp::alibabacloud-credentials-cpp)
else()
    message(STATUS "alibabacloud-credentials-cpp not found, skipping CredentialsCppIntegration")
endif()
