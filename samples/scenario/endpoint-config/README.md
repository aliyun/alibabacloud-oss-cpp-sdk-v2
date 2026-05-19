# Endpoint Configuration Samples

Demonstrates different endpoint modes: custom, dual-stack, internal, accelerate, CName, path-style.

## Samples

| File | Description |
|------|-------------|
| `EndpointModes.cpp` | Switch between 6 endpoint modes via `--mode` flag |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/EndpointModes --region cn-hangzhou --bucket my-bucket --mode dual-stack
./build/EndpointModes --region cn-hangzhou --bucket my-bucket --mode internal
./build/EndpointModes --region cn-hangzhou --bucket my-bucket --mode path-style
```
