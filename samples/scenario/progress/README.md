# Progress Callback Samples

Demonstrates how to monitor upload progress using `ProgressCallback`.

## Samples

| File | Description |
|------|-------------|
| `UploadWithProgress.cpp` | PutObject with real-time progress output |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/UploadWithProgress --region cn-hangzhou --bucket my-bucket
```
