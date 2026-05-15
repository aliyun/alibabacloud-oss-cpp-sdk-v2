# Performance Tests

End-to-end OSS request performance tests built on [Google Benchmark](https://github.com/google/benchmark), measuring real network request throughput and latency.

## Build

```bash
cmake -B build -DBUILD_TESTS=ON -DBUILD_PERF_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target cpp-sdk-perftest
```

## Environment Variables

The following environment variables must be set before running:

| Variable | Required | Description |
|----------|----------|-------------|
| `OSS_TEST_ACCESS_KEY_ID` | Yes | AccessKey ID |
| `OSS_TEST_ACCESS_KEY_SECRET` | Yes | AccessKey Secret |
| `OSS_TEST_REGION` | Yes | Region, e.g. `cn-hangzhou` |
| `OSS_TEST_BUCKET` | Yes | Bucket for performance testing |
| `OSS_TEST_ENDPOINT` | No | Custom endpoint |

## Run

```bash
./build/tests/perf/Release/cpp-sdk-perftest
```

## Available Benchmarks

| Benchmark | Description |
|-----------|-------------|
| `BM_PutObject_Sync_1KB` | Sync PutObject, 1KB object |
| `BM_PutObject_Sync_1MB` | Sync PutObject, 1MB object |
| `BM_PutObject_Sync_4MB` | Sync PutObject, 4MB object |
| `BM_PutObject_Async_1KB_Concurrent/N` | Async concurrent PutObject, 1KB object, N concurrency |
| `BM_PutObject_Async_1MB_Concurrent/N` | Async concurrent PutObject, 1MB object, N concurrency |
| `BM_PutObject_Async_4MB_Concurrent/N` | Async concurrent PutObject, 4MB object, N concurrency |
| `BM_GetObject_Sync_1KB` | Sync GetObject, 1KB object |
| `BM_GetObject_Sync_1MB` | Sync GetObject, 1MB object |
| `BM_GetObject_Sync_4MB` | Sync GetObject, 4MB object |
| `BM_GetObject_Async_1KB_Concurrent/N` | Async concurrent GetObject, 1KB object, N concurrency |
| `BM_GetObject_Async_1MB_Concurrent/N` | Async concurrent GetObject, 1MB object, N concurrency |
| `BM_GetObject_Async_4MB_Concurrent/N` | Async concurrent GetObject, 4MB object, N concurrency |
| `BM_PutObject_Async_Custom_Concurrent` | Async concurrent PutObject, custom object size and concurrency |
| `BM_GetObject_Async_Custom_Concurrent` | Async concurrent GetObject, custom object size and concurrency |
| `BM_PutObject_Sync_Sustained` | Sustained single-thread sync PutObject throughput |
| `BM_PutObject_Async_Sustained` | Sustained single-thread async PutObject throughput |
| `BM_GetObject_Sync_Sustained` | Sustained single-thread sync GetObject throughput |
| `BM_GetObject_Async_Sustained` | Sustained single-thread async GetObject throughput |

Where N is one of the preset concurrency levels: 10, 50, 100, 200.

### Custom Concurrent Tests

`BM_*_Async_Custom_Concurrent` tests require `--concurrency` and optionally `--object_size`. They are skipped if `--concurrency` is not provided.

### Sustained Throughput Test

`BM_*_Sustained` tests measure the maximum QPS a single thread can sustain by continuously calling the API in a loop. The async variants (`BM_*_Async_Sustained`) use `asyncCall` which blocks naturally when the connection pool is saturated — use different `--async_pool_size` values to compare pool sizing impact. The sync variants (`BM_*_Sync_Sustained`) call the blocking API directly, measuring single-connection serial throughput — use different `--sync_pool_size` values to compare. All sustained tests use `--object_size` for the payload.

## Filtering Benchmarks

Use `--benchmark_filter` with a regex to select which benchmarks to run:

```bash
# Run only PutObject benchmarks
./cpp-sdk-perftest --benchmark_filter="BM_PutObject.*"

# Run only sync benchmarks
./cpp-sdk-perftest --benchmark_filter=".*Sync.*"

# Run only 1MB async concurrent benchmarks
./cpp-sdk-perftest --benchmark_filter=".*Async_1MB.*"

# Run only concurrency level 100
./cpp-sdk-perftest --benchmark_filter=".*/100"

# Run GetObject async 1KB with concurrency 50
./cpp-sdk-perftest --benchmark_filter="BM_GetObject_Async_1KB_Concurrent/50"

# List all benchmarks without running
./cpp-sdk-perftest --benchmark_list_tests
```

## Custom Options

| Option | Description |
|--------|-------------|
| `--concurrency <N>` | Concurrency for `BM_*_Custom_Concurrent` tests |
| `--object_size <bytes>` | Object size for `BM_*_Custom_Concurrent` and `BM_*_Sustained` tests (default: 1024) |
| `--sync_pool_size <N>` | Set sync client curl connection pool size (default: 16) |
| `--async_pool_size <N>` | Set async client curl connection pool size (default: 100) |

```bash
# Run with a larger async connection pool
./cpp-sdk-perftest --async_pool_size 256 --benchmark_filter=".*Async.*"

# Run with custom pool sizes for both
./cpp-sdk-perftest --sync_pool_size 32 --async_pool_size 512
```

## Google Benchmark Options

| Option | Description |
|--------|-------------|
| `--benchmark_filter=<regex>` | Filter benchmarks by regex |
| `--benchmark_list_tests` | List benchmark names without running |
| `--benchmark_repetitions=N` | Repeat each benchmark N times, report aggregates |
| `--benchmark_report_aggregates_only=true` | Only report aggregate results (mean/median/stddev) |
| `--benchmark_format=<console\|json\|csv>` | Output format |
| `--benchmark_out=<file>` | Write results to file |
| `--benchmark_min_time=Ns` | Minimum time to run each benchmark |

## Examples

```bash
# Run all benchmarks, repeat 3 times, report aggregates only
./cpp-sdk-perftest \
    --benchmark_repetitions=3 \
    --benchmark_report_aggregates_only=true

# Run PutObject 1KB concurrent benchmarks, output as JSON
./cpp-sdk-perftest \
    --benchmark_filter="BM_PutObject_Async_1KB.*" \
    --benchmark_format=json \
    --benchmark_out=put_1kb_result.json

# Run all async benchmarks with custom pool size
./cpp-sdk-perftest \
    --async_pool_size 512 \
    --benchmark_filter=".*Async.*"

# Run custom concurrent test with 500 concurrency and 2MB objects
./cpp-sdk-perftest \
    --concurrency 500 \
    --object_size 2097152 \
    --benchmark_filter="BM_.*_Async_Custom_Concurrent"

# Run sustained throughput test with different pool sizes
./cpp-sdk-perftest \
    --async_pool_size 64 \
    --object_size 1024 \
    --benchmark_filter="BM_PutObject_Async_Sustained" \
    --benchmark_min_time=30s
```
