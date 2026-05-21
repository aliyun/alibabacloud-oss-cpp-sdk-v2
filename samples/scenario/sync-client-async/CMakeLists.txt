cmake_minimum_required(VERSION 3.15)
project(scenario-sync-client-async)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(AsyncCallOnSyncClient AsyncCallOnSyncClient.cpp)
target_link_libraries(AsyncCallOnSyncClient PRIVATE alibabacloud_oss_v2::oss)

add_executable(AsyncCallbackOnSyncClient AsyncCallbackOnSyncClient.cpp)
target_link_libraries(AsyncCallbackOnSyncClient PRIVATE alibabacloud_oss_v2::oss)
