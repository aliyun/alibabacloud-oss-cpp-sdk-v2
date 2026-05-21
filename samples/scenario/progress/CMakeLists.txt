cmake_minimum_required(VERSION 3.15)
project(scenario-progress)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(UploadWithProgress UploadWithProgress.cpp)
target_link_libraries(UploadWithProgress PRIVATE alibabacloud_oss_v2::oss)
