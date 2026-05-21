cmake_minimum_required(VERSION 3.15)
project(scenario-transport-winhttp)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(WinHttpCustomConfig WinHttpCustomConfig.cpp)
target_link_libraries(WinHttpCustomConfig PRIVATE alibabacloud_oss_v2::oss)
