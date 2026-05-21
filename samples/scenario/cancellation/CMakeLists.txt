cmake_minimum_required(VERSION 3.15)
project(scenario-cancellation)

find_package(alibabacloud_oss_v2 REQUIRED)

set(SAMPLES
    CancelInflightRequest
    RequestTimeout
    CancelBatchRequests
    DisableClientRequests
    GracefulShutdown
)
foreach(SAMPLE ${SAMPLES})
    add_executable(${SAMPLE} ${SAMPLE}.cpp)
    target_link_libraries(${SAMPLE} PRIVATE alibabacloud_oss_v2::oss)
endforeach()
