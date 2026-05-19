cmake_minimum_required(VERSION 3.15)
project(scenario-progress)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

add_executable(UploadWithProgress UploadWithProgress.cpp)
target_link_libraries(UploadWithProgress PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
