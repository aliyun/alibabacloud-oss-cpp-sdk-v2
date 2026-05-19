# Custom Retry Strategy Samples

Demonstrates how to configure retry behavior via `Retryer` and `BackoffDelayer`.

## Samples

| File | Description |
|------|-------------|
| `CustomRetryStrategy.cpp` | FixedDelay, EqualJitter, NopRetryer, retryMaxAttempts |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/CustomRetryStrategy --region cn-hangzhou --bucket my-bucket --strategy fixed
./build/CustomRetryStrategy --region cn-hangzhou --bucket my-bucket --strategy no-retry
```
