cmake_minimum_required(VERSION 3.15)
project(scenario-cancellation)

find_package(alibabacloud-oss-cpp-sdk-v2 REQUIRED)

set(SAMPLES
    CancelInflightRequest
    RequestTimeout
    CancelBatchRequests
)
foreach(SAMPLE ${SAMPLES})
    add_executable(${SAMPLE} ${SAMPLE}.cpp)
    target_link_libraries(${SAMPLE} PRIVATE alibabacloud-oss-cpp-sdk-v2::cpp-sdk-v2)
endforeach()
