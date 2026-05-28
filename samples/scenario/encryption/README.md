# Custom MasterCipher Sample

Demonstrates how to implement a custom `MasterCipher` for `OSSEncryptionClient`.

## Samples

| File | Description |
|------|-------------|
| `CustomMasterCipher.cpp` | Implement a custom key-wrapping cipher and use it with `OSSEncryptionClient` |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2` with `-DENABLE_ENCRYPTION=ON`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/CustomMasterCipher --region cn-hangzhou --bucket my-bucket
```
