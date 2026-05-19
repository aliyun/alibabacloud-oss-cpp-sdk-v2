cmake_minimum_required(VERSION 3.15)
project(scenario-request-body)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(RequestBodyVariants RequestBodyVariants.cpp)
target_link_libraries(RequestBodyVariants PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
