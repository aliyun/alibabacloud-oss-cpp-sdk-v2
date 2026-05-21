cmake_minimum_required(VERSION 3.15)

project(oss-cpp-sdk-v2-samples)

# SAMPLE_FILTER: prefix filter for sample targets
# Examples:
#   -DSAMPLE_FILTER=api              -> all samples under api/
#   -DSAMPLE_FILTER=api/sync         -> all samples under api/sync/
#   -DSAMPLE_FILTER=api/sync/PutObject -> only PutObject sample
# Default: build all samples
set(SAMPLE_FILTER "" CACHE STRING "Prefix filter for sample targets (e.g. api, api/sync, api/sync/PutObject)")

# ── Regular samples (api, paginator, transfer) ──────────────
file(GLOB_RECURSE SAMPLE_SRCS
    "api/*.cpp"
    "paginator/*.cpp"
    "transfer/*.cpp"
)

foreach(SAMPLE_SRC ${SAMPLE_SRCS})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${SAMPLE_SRC})

    # Apply filter
    if(SAMPLE_FILTER AND NOT REL_PATH MATCHES "^${SAMPLE_FILTER}")
        continue()
    endif()

    # Build target name: api/sync/PutObject.cpp -> sample_api_sync_PutObject
    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
    get_filename_component(BASE_NAME ${REL_PATH} NAME_WE)
    string(REPLACE "/" "_" DIR_PREFIX "${REL_DIR}")
    set(TARGET_NAME "sample_${DIR_PREFIX}_${BASE_NAME}")

    add_executable(${TARGET_NAME} ${SAMPLE_SRC})
    target_include_directories(${TARGET_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_link_libraries(${TARGET_NAME} PRIVATE cpp-sdk-v2)
    if(USE_CURL_TRANSPORT)
        target_link_libraries(${TARGET_NAME} PRIVATE ${CURL_LIB})
    endif()
    target_compile_options(${TARGET_NAME}
        PRIVATE "${SDK_COMPILER_FLAGS}")
endforeach()

# ── Scenario samples (samples/scenario/*) are independent projects ──
# They use find_package() for SDK and external dependencies,
# and must be built separately from their own directory.
# See samples/scenario/*/README.md for build instructions.
