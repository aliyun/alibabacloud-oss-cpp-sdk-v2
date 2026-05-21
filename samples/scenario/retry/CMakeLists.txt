cmake_minimum_required(VERSION 3.15)
project(scenario-retry)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(CustomRetryStrategy CustomRetryStrategy.cpp)
target_link_libraries(CustomRetryStrategy PRIVATE alibabacloud_oss_v2::oss)
