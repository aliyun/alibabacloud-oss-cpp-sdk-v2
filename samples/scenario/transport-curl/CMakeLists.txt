cmake_minimum_required(VERSION 3.15)
project(scenario-transport-curl)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(CurlCustomConfig CurlCustomConfig.cpp)
target_link_libraries(CurlCustomConfig PRIVATE alibabacloud_oss_v2::oss)
