# SinkFactory Samples

Demonstrates how to use `SinkFactory` with `ByteWriter` to control where downloaded data is written.

## Samples

| File | Description |
|------|-------------|
| `DownloadToFileWithCRC.cpp` | Download to file with progress reporting and CRC-64 verification |
| `DownloadToMemory.cpp` | Zero-copy download into a pre-allocated memory buffer |
| `ResumableDownload.cpp` | Resumable download that recovers from network interruptions using Range requests |
| `AsyncDownloadWithSinkFactory.cpp` | Async client with SinkFactory, progress, and CRC (thread safety notes) |
| `ParallelRangeDownload.cpp` | Multi-threaded parallel range download into a shared buffer via MemoryWriter |
| `DownloadWithCancellation.cpp` | Cancel download mid-flight, partial data remains usable |

## Key Concepts

- **SinkFactory**: A factory that creates a `ByteWriter` per download attempt. On retries, the factory is called again to produce a fresh writer.
- **ObservableWriter**: Composes a primary writer with observers (progress, CRC) that see the same data without extra copies.
- **MemoryWriter**: Writes directly into a user-provided `uint8_t*` buffer, avoiding intermediate stream copies.
- **ProgressWriteObserver**: Reports incremental download progress via `ProgressCallback`.
- **CRC64WriteObserver**: Computes a running CRC-64 checksum over received data for integrity verification.

## Prerequisites

- Install `alibabacloud-oss-cpp-sdk-v2`
- Set `OSS_ACCESS_KEY_ID` and `OSS_ACCESS_KEY_SECRET` environment variables
- Upload a test object to your bucket

## Build & Run

```bash
cmake -B build .
cmake --build build

./build/DownloadToFileWithCRC --region cn-hangzhou --bucket my-bucket --key my-object
./build/DownloadToMemory --region cn-hangzhou --bucket my-bucket --key my-object
./build/ResumableDownload --region cn-hangzhou --bucket my-bucket --key my-object
./build/AsyncDownloadWithSinkFactory --region cn-hangzhou --bucket my-bucket --key my-object
./build/ParallelRangeDownload --region cn-hangzhou --bucket my-bucket --key my-object --parts 4
./build/DownloadWithCancellation --region cn-hangzhou --bucket my-bucket --key my-object --cancel-after-ms 100
```
