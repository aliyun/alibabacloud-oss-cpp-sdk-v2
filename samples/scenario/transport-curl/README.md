# Curl Transport Customization

Demonstrates how to configure the curl HTTP transport layer.

## Samples

| File | Description |
|------|-------------|
| `CurlCustomConfig.cpp` | Connection pool, timeouts, CA certs, proxy, verbose, requestInterceptor |

## Prerequisites

- SDK built with `USE_CURL_TRANSPORT=ON` (default)
- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/CurlCustomConfig --region cn-hangzhou --bucket my-bucket --verbose
```
