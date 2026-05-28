cmake_minimum_required(VERSION 3.15)
project(scenario-encryption)

find_package(alibabacloud_oss_v2 REQUIRED)

add_executable(CustomMasterCipher CustomMasterCipher.cpp)
target_link_libraries(CustomMasterCipher PRIVATE alibabacloud_oss_v2::oss)
