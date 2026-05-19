# Scenario: Async on Sync Client

Demonstrates using `OSSClient::asyncCall()` and `OSSClient::asyncCallback()` to run
synchronous API calls in parallel via an `Executor`.

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set environment variables: `OSS_ACCESS_KEY_ID`, `OSS_ACCESS_KEY_SECRET`

## Build

```bash
cmake -B build -DCMAKE_PREFIX_PATH=<sdk-install-prefix>
cmake --build build
```

## Samples

| File | Pattern | Description |
|------|---------|-------------|
| AsyncCallOnSyncClient.cpp | `asyncCall()` + `std::future` | Parallel upload and head via futures |
| AsyncCallbackOnSyncClient.cpp | `asyncCallback()` + callback | Batch HeadObject with completion callbacks |

## Run

```bash
./AsyncCallOnSyncClient --region cn-hangzhou --bucket my-bucket
./AsyncCallbackOnSyncClient --region cn-hangzhou --bucket my-bucket
```
