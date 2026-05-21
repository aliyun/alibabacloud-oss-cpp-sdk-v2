# Cancellation Samples

Demonstrates how to cancel in-flight requests using `CancellationToken` and client-level request control.

## Samples

| File | Description |
|------|-------------|
| `CancelInflightRequest.cpp` | Cancel a running upload from another thread |
| `RequestTimeout.cpp` | Set a total request deadline with `cancelAfter(ms)` |
| `CancelBatchRequests.cpp` | Cancel multiple concurrent requests with one token |
| `DisableClientRequests.cpp` | Disable all requests at client level, then re-enable |
| `GracefulShutdown.cpp` | Combine disableRequest() + token cancel for graceful shutdown |

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
./build/DisableClientRequests --region cn-hangzhou --bucket my-bucket
./build/GracefulShutdown --region cn-hangzhou --bucket my-bucket
```
