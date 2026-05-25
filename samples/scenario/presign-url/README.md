# Presigned URL Samples

Demonstrates how to use presigned URLs to upload and download objects without exposing OSS credentials to the client. Each sample runs an App Server (httplib) and a Client in the same process to simulate the full interaction.

## Samples

| File | Description |
|------|-------------|
| `PresignUpload.cpp` | Client requests a presigned PUT URL from App Server, then uploads directly to OSS |
| `PresignMultipartUpload.cpp` | App Server initiates multipart upload and presigns each part; client uploads parts and reports ETags back for completion |
| `PresignDownload.cpp` | Client requests a presigned GET URL from App Server, then downloads directly from OSS |

## Dependencies

- `alibabacloud-oss-cpp-sdk-v2`
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (header-only HTTP server/client library)

## Prerequisites

- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/PresignUpload --region cn-hangzhou --bucket my-bucket
./build/PresignMultipartUpload --region cn-hangzhou --bucket my-bucket
./build/PresignDownload --region cn-hangzhou --bucket my-bucket
```
