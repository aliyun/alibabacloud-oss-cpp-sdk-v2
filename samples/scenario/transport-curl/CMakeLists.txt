cmake_minimum_required(VERSION 3.15)
project(scenario-transport-curl)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(CurlCustomConfig CurlCustomConfig.cpp)
target_link_libraries(CurlCustomConfig PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
