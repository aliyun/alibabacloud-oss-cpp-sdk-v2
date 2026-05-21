cmake_minimum_required(VERSION 3.15)
project(scenario-endpoint-config)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(EndpointModes EndpointModes.cpp)
target_link_libraries(EndpointModes PRIVATE alibabacloud_oss_v2::oss)
