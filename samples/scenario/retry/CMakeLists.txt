cmake_minimum_required(VERSION 3.15)
project(scenario-retry)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(CustomRetryStrategy CustomRetryStrategy.cpp)
target_link_libraries(CustomRetryStrategy PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
