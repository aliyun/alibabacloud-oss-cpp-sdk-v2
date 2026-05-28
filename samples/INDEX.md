# Samples

## Directory Structure

| Directory | Description |
|-----------|-------------|
| [api/sync/](api/sync/INDEX.md) | Sync API samples using `OSSClient` |
| [api/async/](api/async/INDEX.md) | Async API samples using `OSSAsyncClient` |
| [paginator/](paginator/INDEX.md) | Paginator samples using `makePaginator()` |
| [encryption/](encryption/INDEX.md) | Client-side encryption samples using `OSSEncryptionClient` |
| common/ | Shared helpers (`SampleConfig.h`) |
| [scenario/sync-client-async/](scenario/sync-client-async/README.md) | Using `asyncCall()` / `asyncCallback()` on `OSSClient` |
| [scenario/credentials/](scenario/credentials/README.md) | Various credential providers and credentials-cpp integration |
| [scenario/progress/](scenario/progress/README.md) | Upload and download progress tracking |
| [scenario/transport-curl/](scenario/transport-curl/README.md) | Curl HTTP transport customization |
| [scenario/transport-winhttp/](scenario/transport-winhttp/README.md) | WinHTTP transport customization |
| [scenario/retry/](scenario/retry/README.md) | Custom retry strategies and backoff |
| [scenario/request-body/](scenario/request-body/README.md) | `RequestBody` factory variants (String, File, Stream, Memory) |
| [scenario/cancellation/](scenario/cancellation/README.md) | Cancel in-flight requests with `CancellationToken` |
| [scenario/endpoint-config/](scenario/endpoint-config/README.md) | Endpoint modes (dual-stack, internal, accelerate, CName, path-style) |
| [scenario/sink-factory/](scenario/sink-factory/README.md) | Download with SinkFactory (progress, CRC-64, zero-copy MemoryWriter) |
| [scenario/presign-url/](scenario/presign-url/README.md) | Presigned URL upload, multipart upload, and download |
| [scenario/encryption/](scenario/encryption/README.md) | Custom MasterCipher implementation for client-side encryption |

## Build

```bash
# Build all samples
cmake -B build -DBUILD_SAMPLES=ON
cmake --build build --config Release

# Build a subset using SAMPLE_FILTER
cmake -B build -DBUILD_SAMPLES=ON -DSAMPLE_FILTER=api/sync
cmake -B build -DBUILD_SAMPLES=ON -DSAMPLE_FILTER=paginator
cmake -B build -DBUILD_SAMPLES=ON -DSAMPLE_FILTER=api/sync/PutObject
```

## Run

```bash
# All samples require --region; most require --bucket and --key
./sample_api_sync_PutObject --region cn-hangzhou --bucket my-bucket --key my-key
./sample_paginator_ListObjectsV2Paginator --region cn-hangzhou --bucket my-bucket

# Credentials are read from environment variables
export OSS_ACCESS_KEY_ID=<your-ak>
export OSS_ACCESS_KEY_SECRET=<your-sk>
```

## Scenario Samples

Scenario samples under `scenario/` are **independent projects** with their own `CMakeLists.txt`.
They are NOT built by `BUILD_SAMPLES=ON`. Build them separately:

**Option A: Using vcpkg**

```bash
vcpkg install alibabacloud-oss-cpp-sdk-v2

cd samples/scenario/sync-client-async
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

**Option B: From source**

```bash
# 1. Build and install the SDK
cmake -B build -DCMAKE_INSTALL_PREFIX=<sdk-install-prefix>
cmake --build build --config Release
cmake --install build --config Release

# 2. Build the scenario sample
cd samples/scenario/sync-client-async
cmake -B build -DCMAKE_PREFIX_PATH=<sdk-install-prefix>
cmake --build build
```
