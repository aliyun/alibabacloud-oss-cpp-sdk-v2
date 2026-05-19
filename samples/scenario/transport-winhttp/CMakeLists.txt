cmake_minimum_required(VERSION 3.15)
project(scenario-transport-winhttp)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(WinHttpCustomConfig WinHttpCustomConfig.cpp)
target_link_libraries(WinHttpCustomConfig PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
