# Cancellation Samples

Demonstrates how to cancel in-flight requests using `CancellationToken`.

## Samples

| File | Description |
|------|-------------|
| `CancelInflightRequest.cpp` | Cancel a running upload from another thread |
| `RequestTimeout.cpp` | Set a total request deadline with `cancelAfter(ms)` |
| `CancelBatchRequests.cpp` | Cancel multiple concurrent requests with one token |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/CancelInflightRequest --region cn-hangzhou --bucket my-bucket
./build/RequestTimeout --region cn-hangzhou --bucket my-bucket --timeout-ms 3000
./build/CancelBatchRequests --region cn-hangzhou --bucket my-bucket
```
