cmake_minimum_required(VERSION 3.15)
project(scenario-sync-client-async)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(AsyncCallOnSyncClient AsyncCallOnSyncClient.cpp)
target_link_libraries(AsyncCallOnSyncClient PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)

add_executable(AsyncCallbackOnSyncClient AsyncCallbackOnSyncClient.cpp)
target_link_libraries(AsyncCallbackOnSyncClient PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
