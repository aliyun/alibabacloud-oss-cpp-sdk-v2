# Progress Callback Samples

Demonstrates how to monitor upload and download progress.

## Samples

| File | Description |
|------|-------------|
| `UploadWithProgress.cpp` | PutObject with real-time progress output via `ProgressCallback` |
| `DownloadToFileWithProgress.cpp` | getObjectToFile with progress callback (simplest download progress) |
| `DownloadWithProgress.cpp` | GetObject with progress tracking via SinkFactory and `ProgressWriteObserver` |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/UploadWithProgress --region cn-hangzhou --bucket my-bucket
./build/DownloadToFileWithProgress --region cn-hangzhou --bucket my-bucket --key my-object
./build/DownloadWithProgress --region cn-hangzhou --bucket my-bucket --key my-object
```
