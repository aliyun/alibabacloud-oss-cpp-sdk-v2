# WinHTTP Transport Customization

Demonstrates how to configure the WinHTTP transport layer (Windows only).

## Samples

| File | Description |
|------|-------------|
| `WinHttpCustomConfig.cpp` | Connection pool, timeouts, proxy, SSL settings |

## Prerequisites

- SDK built with `USE_WINHTTP_TRANSPORT=ON`
- Windows platform
- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
.\build\WinHttpCustomConfig --region cn-hangzhou --bucket my-bucket
```
