# RequestBody Variants

Demonstrates the four `RequestBody` factory methods for constructing request bodies.

## Samples

| File | Description |
|------|-------------|
| `RequestBodyVariants.cpp` | FromString, FromFile, FromStream, FromMemory |

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables

## Build & Run

```bash
cmake -B build .
cmake --build build
./build/RequestBodyVariants --region cn-hangzhou --bucket my-bucket
```
