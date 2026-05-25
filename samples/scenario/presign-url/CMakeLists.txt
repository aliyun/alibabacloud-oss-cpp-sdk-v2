cmake_minimum_required(VERSION 3.15)
project(scenario-presign-url)

find_package(alibabacloud_oss_v2 REQUIRED)
find_path(HTTPLIB_INCLUDE_DIR httplib.h)

add_executable(PresignUpload PresignUpload.cpp)
target_link_libraries(PresignUpload PRIVATE alibabacloud_oss_v2::oss)
target_include_directories(PresignUpload PRIVATE ${HTTPLIB_INCLUDE_DIR})

add_executable(PresignMultipartUpload PresignMultipartUpload.cpp)
target_link_libraries(PresignMultipartUpload PRIVATE alibabacloud_oss_v2::oss)
target_include_directories(PresignMultipartUpload PRIVATE ${HTTPLIB_INCLUDE_DIR})

add_executable(PresignDownload PresignDownload.cpp)
target_link_libraries(PresignDownload PRIVATE alibabacloud_oss_v2::oss)
target_include_directories(PresignDownload PRIVATE ${HTTPLIB_INCLUDE_DIR})
