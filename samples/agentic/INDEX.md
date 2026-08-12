# Agentic Samples

Agentic bucket samples using `OSSAgenticBucketClient` (sync) and `OSSAsyncAgenticBucketClient` (async).
Each file is a standalone executable.

The agentic client resolves a logical bucket prefix into the physical bucket name
`{prefix}-{accountId}-{region}-ab-apsr` for signing and host construction, so every sample
requires `--account-id` in addition to `--region`. An invalid (non-digit) account id surfaces
as an `IllegalArgument` error at call time, not at construction.

## Error Handling

All samples follow this pattern:
```cpp
auto outcome = client.xxx(agentic::models::XxxRequest()...);
if (!outcome.has_value()) {
    auto& e = outcome.error();
    std::cerr << "Fail, code: " << e.getCode()
              << ", message: " << e.getMessage()
              << ", ec: " << e.getEC()
              << ", requestId: " << e.getRequestId()
              << ", requestTarget: " << e.getRequestTarget() << std::endl;
    return 1;
}
auto& result = outcome.value();
// use result...
```

## Operations

| Use Case | Sync File | Async File | Key API |
|----------|-----------|------------|---------|
| Create an agentic bucket | sync/CreateAgenticBucket.cpp | async/CreateAgenticBucket.cpp | `createAgenticBucket()` / `createAgenticBucketAsync()` |
| Delete an agentic bucket | sync/DeleteAgenticBucket.cpp | async/DeleteAgenticBucket.cpp | `deleteAgenticBucket()` / `deleteAgenticBucketAsync()` |
| Query an agentic bucket | sync/GetAgenticBucket.cpp | async/GetAgenticBucket.cpp | `getAgenticBucket()` / `getAgenticBucketAsync()` |
| List agentic buckets | sync/ListAgenticBuckets.cpp | async/ListAgenticBuckets.cpp | `listAgenticBuckets()` / `listAgenticBucketsAsync()` |
| Update agentic bucket status | sync/PutAgenticBucketStatus.cpp | async/PutAgenticBucketStatus.cpp | `putAgenticBucketStatus()` / `putAgenticBucketStatusAsync()` |
| List bucket spaces | sync/ListBucketSpaces.cpp | async/ListBucketSpaces.cpp | `listBucketSpaces()` / `listBucketSpacesAsync()` |

## Bucket Space Object Access

`BucketSpace.cpp` demonstrates object operations on a bucket space in two usage modes:

- **Mode 1 - dedicated scoped client**: `makeBucketSpaceClient()` / `makeAsyncBucketSpaceClient()`
  return a standard `OSSClient` / `OSSAsyncClient` that internally resolves the logical space
  prefix into `{prefix}-{accountId}-{region}-bs-apsr`. Pass only the prefix as the bucket.
- **Mode 2 - existing client + helper**: `BucketSpaceHelper::toBucketName(prefix)` resolves the
  physical bucket name manually, which is then passed to a regular client whose endpoint you
  manage yourself (e.g. internal or CName).

| Use Case | Sync File | Async File | Key API |
|----------|-----------|------------|---------|
| Create a bucket space | sync/CreateBucketSpace.cpp | async/CreateBucketSpace.cpp | `putBucket()` with `setAgenticBucket()` |
| Bucket space object put/get (two modes) | sync/BucketSpace.cpp | async/BucketSpace.cpp | `makeBucketSpaceClient()` / `BucketSpaceHelper::toBucketName()` |

Creating a bucket space is the only bucket space operation that needs the parent agentic bucket:
pass its full name `{bucket}-{accountId}-{region}-ab-apsr` via `setAgenticBucket()`. Object
operations inside an existing space do not need it.

## Run

```bash
# All samples require --region and --account-id; ListAgenticBuckets does not need --bucket.
./sample_agentic_sync_CreateAgenticBucket --region cn-hangzhou --account-id 1234567890 --bucket my-bucket
./sample_agentic_sync_GetAgenticBucket --region cn-hangzhou --account-id 1234567890 --bucket my-bucket
./sample_agentic_sync_ListAgenticBuckets --region cn-hangzhou --account-id 1234567890
./sample_agentic_sync_PutAgenticBucketStatus --region cn-hangzhou --account-id 1234567890 --bucket my-bucket
./sample_agentic_sync_ListBucketSpaces --region cn-hangzhou --account-id 1234567890 --bucket my-bucket
./sample_agentic_sync_DeleteAgenticBucket --region cn-hangzhou --account-id 1234567890 --bucket my-bucket

# Creating a bucket space needs --bucket (space prefix) and --agentic-bucket (parent bucket prefix).
./sample_agentic_sync_CreateBucketSpace --region cn-hangzhou --account-id 1234567890 --bucket my-sandbox --agentic-bucket my-bucket

# Bucket space object access (both modes) needs --bucket (space prefix) and --key.
./sample_agentic_sync_BucketSpace --region cn-hangzhou --account-id 1234567890 --bucket my-sandbox --key test.txt

# Async equivalents share the same flags, e.g.:
./sample_agentic_async_ListAgenticBuckets --region cn-hangzhou --account-id 1234567890
./sample_agentic_async_BucketSpace --region cn-hangzhou --account-id 1234567890 --bucket my-sandbox --key test.txt

# Credentials are read from environment variables
export OSS_ACCESS_KEY_ID=<your-ak>
export OSS_ACCESS_KEY_SECRET=<your-sk>
```
