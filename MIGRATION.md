# Migration Guide

[Chinese](MIGRATION_CN.md)

This guide describes how to upgrade OSS SDK for C++ from V1 ([aliyun-oss-cpp-sdk](https://github.com/aliyun/aliyun-oss-cpp-sdk)) to V2.

## Major breaking changes

- V2 requires C++17 or later.
- `InitializeSdk()` / `ShutdownSdk()` are no longer required.
- Namespace changes from `AlibabaCloud::OSS` to `alibabacloud::oss2`.
- Client constructors are changed. Credentials, endpoint, region, and transport are configured through `ClientConfiguration`.
- Convenience overloads are removed. Each operation now accepts a typed request object.
- Method names are changed from PascalCase to camelCase.
- V2 defaults to Signature V4, which requires `region`.
- V2 uses an `Outcome` interface compatible with `std::expected`.
- Upload body changes from `std::shared_ptr<std::iostream>` to `RequestBody`.
- Response body factory changes from `IOStreamFactory` to `SinkFactory`.
- Async APIs are redesigned.
- `getObject()` streaming no longer verifies CRC-64.

## Minimum C++ version

V2 requires C++17 or later. V1 required C++11.

## Include paths and namespaces

V2 uses a new repository and reorganized header paths. The namespace changes from `AlibabaCloud::OSS` to `alibabacloud::oss2`.

| Module | V1 | V2 |
|:-------|:---|:---|
| Core | `alibabacloud/oss/OssClient.h` | `alibabacloud/oss2/OSSClient.h` |
| Core (async) | -- | `alibabacloud/oss2/OSSAsyncClient.h` |
| Configuration | `alibabacloud/oss/client/ClientConfiguration.h` | `alibabacloud/oss2/ClientConfiguration.h` |
| Credentials | `alibabacloud/oss/auth/CredentialsProvider.h` | `alibabacloud/oss2/credentials/CredentialsProvider.h` |
| Models | `alibabacloud/oss/model/<Name>.h` | `alibabacloud/oss2/models/<Category>.h` |
| Forward declarations | `alibabacloud/oss/OssFwd.h` | `alibabacloud/oss2/OSSFwd.h` |
| Encryption | `alibabacloud/oss/encryption/*.h` | `alibabacloud/oss2/crypto/*.h` |

Examples

```cpp
// v1
#include <alibabacloud/oss/OssClient.h>
using namespace AlibabaCloud::OSS;
```

```cpp
// v2
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
namespace oss = alibabacloud::oss2;
```

## SDK initialization

V1 required calling `InitializeSdk()` before use and `ShutdownSdk()` on exit. V2 removes this requirement.

```cpp
// v1
AlibabaCloud::OSS::InitializeSdk();
// ... use SDK ...
AlibabaCloud::OSS::ShutdownSdk();
```

```cpp
// v2
// No initialization or shutdown needed
```

## Configuration

V2 unifies all configuration into `ClientConfiguration`. Credentials and endpoint are no longer passed as constructor arguments -- they are set via `ClientConfiguration` fields.

V2 uses `std::optional` for optional fields, and unset fields resolve to sensible defaults.

Examples

```cpp
// v1
#include <alibabacloud/oss/OssClient.h>
using namespace AlibabaCloud::OSS;

ClientConfiguration conf;
conf.connectTimeoutMs = 20000;
conf.requestTimeoutMs = 60000;
conf.verifySSL = false;

auto credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();

OssClient client("oss-cn-hangzhou.aliyuncs.com", credentialsProvider, conf);
```

```cpp
// v2
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
namespace oss = alibabacloud::oss2;

auto conf = oss::ClientConfiguration::loadDefault();
conf.region = "cn-hangzhou";
conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
conf.connectTimeout = 20000;       // milliseconds
conf.readWriteTimeout = 60000;     // milliseconds
conf.insecureSkipVerify = true;

oss::OSSClient client(conf);
```

Configuration parameters

| V1 | V2 | Notes |
|:---|:---|:------|
| `endpoint` (constructor) | `ClientConfiguration::endpoint` | V2 can derive endpoint from `region` |
| `accessKeyId`/`accessKeySecret` (constructor) | `ClientConfiguration::credentialsProvider` | Always use a provider |
| -- | `ClientConfiguration::region` | New in V2 |
| `connectTimeoutMs` | `connectTimeout` | Same unit (ms) |
| `requestTimeoutMs` | `readWriteTimeout` | Renamed |
| `verifySSL` | `insecureSkipVerify` | Inverted logic: v1 true=verify, v2 true=skip |
| `isCname` | `useCName` | Same |
| `isPathStyle` | `usePathStyle` | Same |
| `enableCrc64` | `disableUploadCRC64Check` / `disableDownloadCRC64Check` | Inverted; V2 enables CRC64 by default, use these flags to explicitly disable for upload/download separately |
| `enableDateSkewAdjustment` | `disableClockSkewCorrection` | Inverted |
| `signatureVersion` | `signatureVersion` | V1 defaults to "v1", V2 defaults to "v4" (requires `region`) |
| `retryStrategy` | `retryer` / `retryMaxAttempts` | V2 has built-in retry with backoff |
| `proxyHost`/`proxyPort`/... | `proxyHost` | V2 uses a single URL string |
| `httpClient` | `httpTransport` | Different interface |

## Create a client

V2 changes the constructor to accept only `ClientConfiguration`. Endpoint and credentials are configured in the configuration object.

```cpp
// v1
OssClient client("oss-cn-hangzhou.aliyuncs.com", "ak", "sk", conf);
```

```cpp
// v2
auto conf = oss::ClientConfiguration::loadDefault();
conf.region = "cn-hangzhou";
conf.credentialsProvider = std::make_shared<oss::StaticCredentialsProvider>("ak", "sk");
oss::OSSClient client(conf);
```

## Call API operations

V2 adopts a uniform pattern: `camelCase` method names, every operation takes a single `<OperationName>Request` and returns an `<OperationName>Outcome` (which is `Outcome<Result, Error>`). The convenience overloads from V1 (e.g., `PutObject(bucket, key, content)`) are removed.

V2 also uses `set*` builder-style methods on request objects.

```
OutcomeType operationName(const models::OperationNameRequest& request, const OperationOptions* options = nullptr)
```

Examples

```cpp
// v1
auto outcome = client.PutObject("mybucket", "mykey", 
    std::make_shared<std::stringstream>("hello"));
if (outcome.isSuccess()) {
    // use outcome.result()
} else {
    auto& err = outcome.error();
    std::cerr << err.Code() << ": " << err.Message() << std::endl;
}
```

```cpp
// v2
auto outcome = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("mybucket")
        .setKey("mykey")
        .setBody(oss::RequestBody::fromString("hello")));
if (outcome.has_value()) {
    // use outcome.value() or *outcome
} else {
    auto& err = outcome.error();
    std::cerr << err.getCode() << ": " << err.getMessage() << std::endl;
}
```

Method name changes (selected examples)

Most methods simply change from PascalCase to camelCase. A few are also renamed:

| V1 | V2 | Notes |
|:---|:---|:------|
| `PutObject()` | `putObject()` | Case only |
| `GetObject()` | `getObject()` | Case only |
| `HeadObject()` | `headObject()` | Case only |
| `CreateBucket()` | `putBucket()` | Renamed |
| `DeleteObjects()` | `deleteMultipleObjects()` | Renamed |
| `SetObjectAcl()` | `putObjectAcl()` | Renamed |
| `CreateSymlink()` | `putSymlink()` | Renamed |

## Error handling

V1 uses an `Outcome` type with `isSuccess()`, `result()` and `error()` methods. V2 uses `Outcome<Result, Error>` whose interface is compatible with `std::expected`.

| | V1 | V2 |
|:---|:---|:---|
| Return type | `Outcome` | `Outcome<Result, Error>` |
| Check success | `isSuccess()` | `has_value()` |
| Get result | `result()` | `value()` or `operator*` |
| Get error | `error()` | `error()` |
| Error code | `error().Code()` | `error().getCode()` |
| Error message | `error().Message()` | `error().getMessage()` |
| Request ID | `error().RequestId()` | `error().getRequestId()` |
| Error category | -- | `error().getEC()` (`std::error_code`) |
| Request target | -- | `error().getRequestTarget()` |

```cpp
// v1
auto outcome = client.GetObject(request);
if (outcome.isSuccess()) {
    auto& result = outcome.result();
    // use result.Content()
} else {
    std::cerr << outcome.error().Code() << ": " << outcome.error().Message() << std::endl;
}
```

```cpp
// v2
auto outcome = client.getObject(request);
if (!outcome.has_value()) {
    auto& err = outcome.error();
    std::cerr << "Code: " << err.getCode() << std::endl;
    std::cerr << "Message: " << err.getMessage() << std::endl;
    std::cerr << "EC: " << err.getEC() << std::endl;
    std::cerr << "Request ID: " << err.getRequestId() << std::endl;
    std::cerr << "Request Target: " << err.getRequestTarget() << std::endl;
    return;
}
auto& result = outcome.value();
// use result.getBody()
```

When built with `-DUSE_STD_EXPECTED=ON` (C++23), `Outcome` becomes a type alias for `std::expected`, enabling monadic operations like `.and_then()`, `.transform()`, and `.or_else()`.

V2 also provides a legacy interface (`isSuccess()` / `getResult()` / `getError()`) for compatibility with V1 migration. Note that the legacy interface is not available in `std::expected` mode.

## Request body

V1 uses `std::shared_ptr<std::iostream>` as the upload body for all operations. V2 replaces this with a flexible `RequestBody` namespace that provides four factory methods, each backed by a dedicated content type. Choose the one that best matches your data source:

| Factory method | Underlying type | Ownership | Description |
|:---------------|:----------------|:----------|:------------|
| `RequestBody::fromString(data)` | `StringContent` | Owning (copy/move) | Copies or moves a `std::string` into the body. Safe for all use cases. |
| `RequestBody::fromFile(path)` | `FileContent` | Owning (path) | Opens and reads from a file path. Reopens the file on each retry attempt. |
| `RequestBody::fromStream(stream)` | `StreamContent` | Shared (`shared_ptr`) | Wraps a `std::shared_ptr<std::istream>`. Returns `EmptyContent` if null. |
| `RequestBody::fromMemory(data, len)` | `MemoryContent` | Non-owning (zero-copy) | References existing memory without copying. Caller must keep data alive until the request completes. |

```cpp
// v1
auto content = std::make_shared<std::stringstream>("data");
auto outcome = client.PutObject("bucket", "key", content);

// From file
auto outcome2 = client.PutObject("bucket", "key", "/path/to/file");
```

```cpp
// v2 -- fromString: owns a copy of the data
auto outcome = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::fromString("hello world")));

// v2 -- fromFile: reads from a file path, reopens on retry
auto outcome2 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::fromFile("/path/to/data.bin")));

// v2 -- fromStream: wraps a shared istream
auto ifs = std::make_shared<std::ifstream>("data.bin", std::ios::binary);
auto outcome3 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::fromStream(ifs)));

// v2 -- fromMemory: zero-copy, non-owning reference
const char* buf = getBuffer();
size_t bufLen = getBufferSize();
auto outcome4 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::fromMemory(buf, bufLen)));
```

V2 also provides `putObjectFromFile()` as a convenience method that takes a file path directly:

```cpp
auto outcome = client.putObjectFromFile(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    "/path/to/file");
```

## Response body

Both V1 and V2 return download data as a `std::shared_ptr<std::iostream>` by default. Both also support a custom response factory to control how the response body is received, but the interface differs:

| | V1 | V2 |
|:---|:---|:---|
| Get body | `result.Content()` | `result.getBody()` |
| Body type | `std::shared_ptr<std::iostream>` | `std::shared_ptr<std::iostream>` |
| Custom factory | `request.setResponseStreamFactory(factory)` | `request.setSinkFactory(factory)` |
| Factory type | `IOStreamFactory` = `std::function<std::shared_ptr<std::iostream>()>` | `SinkFactory` with `std::function<std::shared_ptr<ByteWriter>(int64_t size, const HeaderCollection& headers)>` |
| Factory parameters | None | Content length + response headers |
| Retry support | -- | `factory.isOneShot` flag |

```cpp
// v1 -- read from iostream
auto outcome = client.GetObject(request);
if (outcome.isSuccess()) {
    auto& content = outcome.result().Content();
    std::string data(std::istreambuf_iterator<char>(*content), {});
}

// v1 -- custom response stream factory
request.setResponseStreamFactory([&]() {
    return std::make_shared<std::fstream>("/path/to/file",
        std::ios::out | std::ios::binary);
});
```

```cpp
// v2 -- read from iostream
auto outcome = client.getObject(request);
if (outcome.has_value()) {
    auto& body = outcome->getBody();
    std::string data(std::istreambuf_iterator<char>(*body), {});
}

// v2 -- custom sink (zero-copy download into a user-provided buffer)
std::vector<uint8_t> buf(4 * 1024 * 1024);
oss::SinkFactory factory;
factory.supplier = [&buf](std::int64_t, const oss::HeaderCollection&)
    -> std::shared_ptr<oss::ByteWriter> {
    return std::make_shared<oss::MemoryWriter>(buf.data(), buf.size());
};
factory.isOneShot = false;  // supports retry

auto outcome2 = client.getObject(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setSinkFactory(factory));
```

## Request cancellation

V1 only supports client-level cancellation. V2 adds per-request cancellation via `CancellationToken`.

| Level | V1 | V2 |
|:------|:---|:---|
| Per-request | -- | `CancellationToken` via `OperationOptions` |
| Client-level | `client.DisableRequest()` / `client.EnableRequest()` | `client.disableRequest()` / `client.enableRequest()` |

Per-request cancellation uses `CancellationTokenSource` to create a token. The token can be canceled immediately or after a deadline.

```cpp
// v2 -- cancel a single request from another thread
auto cts = oss::CancellationTokenSource::create();

oss::OperationOptions opts;
opts.cancellationToken = cts->getToken();

// launch request in another thread
auto future = std::async([&]() {
    return client.getObject(request, &opts);
});

// cancel from the current thread
cts->cancel();

// or set a deadline-based cancellation
auto cts2 = oss::CancellationTokenSource::create();
cts2->cancelAfter(std::chrono::seconds(30));
opts.cancellationToken = cts2->getToken();
```

Client-level cancellation disables all requests on a client instance. New requests fail immediately until re-enabled. Note that `disableRequest()` only prevents new HTTP calls from being sent; if you are also using per-request `CancellationToken`, you must cancel those tokens explicitly to abort requests that are already in-flight.

```cpp
// v2 -- cancel all requests on a client (including those with CancellationToken)
client.disableRequest();       // prevent new requests and abort pending ones
cts->cancel();                 // also cancel any in-flight request holding this token

// ... later re-enable
client.enableRequest();
```

## Pre-signed URLs

V1 uses `GeneratePresignedUrl()` that returns a URL string. V2 uses `presign()` with typed request overloads that return a `PresignResult` containing the URL, HTTP method, expiration time, and signed headers.

```cpp
// v1
auto outcome = client.GeneratePresignedUrl("bucket", "key", 3600, Http::Method::Get);
if (outcome.isSuccess()) {
    std::cout << outcome.result() << std::endl;
}
```

```cpp
// v2
oss::models::PresignOptions opts;
opts.setExpiresDuration(std::chrono::seconds(3600));

auto outcome = client.presign(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    &opts);
if (outcome.has_value()) {
    std::cout << "URL: " << outcome->getUrl() << std::endl;
    std::cout << "Method: " << outcome->getMethod() << std::endl;
}
```

## Async operations

V1 provides separate `*Async()` and `*Callable()` methods for each operation on `OssClient`. V2 offers two approaches:

| | V1 `OssClient` | V2 `OSSClient` | V2 `OSSAsyncClient` |
|:---|:---|:---|:---|
| Header | `alibabacloud/oss/OssClient.h` | `alibabacloud/oss2/OSSClient.h` | `alibabacloud/oss2/OSSAsyncClient.h` |
| HTTP transport | Sync (thread pool wrapper) | Sync (thread pool wrapper) | Native async HTTP |
| Requires `Executor` | No (internal) | Yes (`conf.executor`) | No |
| Future-based | `GetObjectCallable()` | `asyncCall(request)` | `asyncCall(request)` |
| Callback-based | `GetObjectAsync(req, cb)` | `asyncCallback(req, cb)` | `getObjectAsync(req, cb)` |
| Method style | Per-operation `*Async()`/`*Callable()` | Generic template for all operations | Per-operation `*Async()` + generic `asyncCall()` |

### OSSClient async (sync client + thread pool)

V2 `OSSClient` uses generic template methods `asyncCall()` and `asyncCallback()` that work with any operation, replacing V1's per-operation methods.

```cpp
// v1 - per-operation callback
client.GetObjectAsync(request, 
    [](const OssClient*, const GetObjectRequest& req, const GetObjectOutcome& outcome, 
       const std::shared_ptr<const AsyncCallerContext>&) {
        if (outcome.isSuccess()) { /* ... */ }
    });

// v1 - per-operation future
auto task = client.GetObjectCallable(request);
auto outcome = task.get();
```

```cpp
// v2 OSSClient - requires executor
conf.executor = std::make_shared<oss::ThreadPoolExecutor>(4);
oss::OSSClient client(conf);

// future-based (generic template)
auto future = client.asyncCall(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"));
auto outcome = future.get();

// callback-based (generic template)
client.asyncCallback(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    [](const oss::OSSClient*, const oss::models::GetObjectRequest&, 
       const oss::GetObjectOutcome& outcome) {
        if (outcome.has_value()) { /* ... */ }
    });
```

### OSSAsyncClient (native async HTTP)

V2 also provides `OSSAsyncClient` for fully asynchronous operations with native async HTTP transport. No `Executor` is needed.

```cpp
// v2 OSSAsyncClient
oss::OSSAsyncClient asyncClient(conf);

// per-operation callback
asyncClient.getObjectAsync(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    [](oss::GetObjectOutcome outcome) {
        if (outcome.has_value()) { /* ... */ }
    });

// future-based (generic template)
auto future = asyncClient.asyncCall(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"));
auto outcome = future.get();
```

## Paginator

V1 requires manual pagination with markers. V2 provides `makePaginator()` for automatic page iteration.

```cpp
// v1 - manual pagination
std::string marker;
do {
    ListObjectsRequest request("mybucket");
    if (!marker.empty()) request.setMarker(marker);
    auto outcome = client.ListObjects(request);
    if (!outcome.isSuccess()) break;
    for (auto& obj : outcome.result().ObjectSummarys()) {
        // process obj
    }
    if (outcome.result().IsTruncated()) {
        marker = outcome.result().NextMarker();
    } else {
        break;
    }
} while (true);
```

```cpp
// v2 - automatic pagination
auto paginator = oss::makePaginator(client,
    oss::models::ListObjectsRequest()
        .setBucket("mybucket")
        .setMaxKeys(100));
while (paginator.hasNext()) {
    auto outcome = paginator.nextPage();
    if (!outcome.has_value()) break;
    for (auto& obj : outcome->getContents()) {
        // process obj
    }
}
```

Supported paginators: `ListBucketsRequest`, `ListObjectsRequest`, `ListObjectsV2Request`, `ListObjectVersionsRequest`, `ListMultipartUploadsRequest`, `ListPartsRequest`.

## Resumable transfer

V1 provides `ResumableUploadObject()`, `ResumableDownloadObject()`, and `ResumableCopyObject()`. V2 will provide `Uploader`, `Downloader`, and `Copier` to support these capabilities in the future. Currently, `getObjectToFile()` supports automatic resume on network failure using Range requests.

| Scenario | V1 | V2 |
|:---------|:---|:---|
| Upload file | `PutObject(bucket, key, filePath)` | `putObjectFromFile(request, filePath)` |
| Download to file | `GetObject(bucket, key, filePath)` | `getObjectToFile(request, filePath)` |
| Upload large file (resumable) | `ResumableUploadObject(request)` | `Uploader` (TBD) |
| Download large file (resumable) | `ResumableDownloadObject(request)` | `Downloader` (TBD); currently `getObjectToFile()` auto-resumes via Range requests |

## Convenience methods

V1 provides many convenience overloads (e.g., `GetObject(bucket, key)`, `PutObject(bucket, key, content)`). V2 removes these in favor of a single method per operation, using builder-style request objects. V2 also provides some extension methods:

| V1 | V2 |
|:---|:---|
| `PutObject(bucket, key, filePath)` | `putObjectFromFile(request, filePath)` |
| `GetObject(bucket, key, filePath)` | `getObjectToFile(request, filePath)` |
| `DoesBucketExist(bucket)` | `isBucketExist(bucket)` |
| `DoesObjectExist(bucket, key)` | `isObjectExist(bucket, key)` |

## Client-side encryption

V1 uses `OssEncryptionClient` (derived from `OssClient`) with `ContentCryptoMaterial` and `EncryptionMaterials`. V2 uses `OSSEncryptionClient` with a simplified `MasterCipher` interface and `EncryptionConfiguration`.

```cpp
// v1
#include <alibabacloud/oss/OssEncryptionClient.h>
#include <alibabacloud/oss/encryption/CryptoConfiguration.h>
#include <alibabacloud/oss/encryption/EncryptionMaterials.h>

auto materials = std::make_shared<SimpleRSAEncryptionMaterials>(
    publicKey, privateKey, description);
CryptoConfiguration cryptoConf;
OssEncryptionClient eclient(endpoint, credProvider, conf, materials, cryptoConf);
auto outcome = eclient.PutObject(request);
```

```cpp
// v2
#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/RsaMasterCipher.h"

auto masterCipher = oss::crypto::makeRsaMasterCipher(publicKeyPem, privateKeyPem, description);
oss::crypto::EncryptionConfiguration encConfig;
encConfig.masterCipher = masterCipher;
oss::OSSEncryptionClient eclient(conf, std::move(encConfig));

auto outcome = eclient.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::fromString("data")));
```

V2 `OSSEncryptionClient` supports: `putObject`, `getObject`, `headObject`, `getObjectMeta`, `initiateMultipartUpload`, `uploadPart`, `completeMultipartUpload`, `abortMultipartUpload`, `listParts`, and `unwrap()` to access the underlying `OSSClient` for non-encryption operations.

## Retry

Both V1 and V2 enable automatic retry by default (max 3 attempts). V2 uses `FullJitterBackoff` for exponential backoff with jitter.

| | V1 | V2 |
|:---|:---|:---|
| Default max attempts | 3 | 3 |
| Backoff strategy | `scaleFactor` based | `FullJitterBackoff` (exponential with jitter) |
| Configuration | `ClientConfiguration::retryStrategy` | `ClientConfiguration::retryer` / `retryMaxAttempts` |
| Per-request override | -- | `OperationOptions::retryMaxAttempts` |
| Interface | `RetryStrategy` (inherit `shouldRetry` + `calcDelayTimeMs`) | `Retryer` |

## CRC-64

CRC-64 is used to verify data integrity during upload and download. Both V1 and V2 enable CRC-64 by default for uploads.

V1 also enables CRC-64 for `getObject()` streaming downloads — the HTTP client computes CRC-64 during receive and verifies it against the server-returned `x-oss-hash-crc64ecma` header. V2's `getObject()` does **not** perform CRC-64 verification for streaming reads; if you need CRC-64 verification on download, use `getObjectToFile()` or verify manually with `SinkFactory` + `CRC64WriteObserver`.

| Operation | V1 | V2 |
|:----------|:---|:---|
| `putObject` | CRC-64 enabled | CRC-64 enabled |
| `appendObject` | CRC-64 enabled | CRC-64 enabled |
| `uploadPart` | CRC-64 enabled | CRC-64 enabled |
| `getObject` (streaming) | CRC-64 enabled | No CRC-64 |
| `getObjectToFile` / `ResumableDownloadObject` | CRC-64 enabled | CRC-64 enabled |
| Disable upload CRC | `enableCrc64 = false` | `disableUploadCRC64Check = true` |
| Disable download CRC | `enableCrc64 = false` | `disableDownloadCRC64Check = true` |

To manually verify CRC-64 for streaming downloads with V2's `getObject()`, use `SinkFactory` with `CRC64WriteObserver`:

```cpp
// v2 -- manual CRC-64 verification for getObject streaming download
auto crc = std::make_shared<oss::CRC64WriteObserver>();

oss::SinkFactory factory;
factory.isOneShot = false;
factory.supplier = [&crc](std::int64_t, const oss::HeaderCollection&)
    -> std::shared_ptr<oss::ByteWriter> {
    // reset CRC state so retries recompute from scratch
    crc->reset();
    auto file = std::make_shared<oss::OStreamWriter>(
        std::make_shared<std::ofstream>("local.dat", std::ios::binary | std::ios::trunc));
    return std::make_shared<oss::ObservableWriter>(file, crc);
};

auto outcome = client.getObject(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setSinkFactory(factory));

if (outcome.has_value()) {
    auto serverCrc = outcome->getHashCrc64ecmaAsUint64();
    if (serverCrc != 0 && crc->crc() != serverCrc) {
        // CRC mismatch, data may be corrupted
    }
}
```

## Credentials

V2 provides equivalent credential providers with renamed classes:

| V1 | V2 |
|:---|:---|
| `SimpleCredentialsProvider` | `StaticCredentialsProvider` |
| `EnvironmentVariableCredentialsProvider` | `EnvironmentVariableCredentialsProvider` |
| `CredentialsProvider` (interface) | `CredentialsProvider` (interface) |
| -- | `CredentialsProviderFunc` (new) |
| -- | `AnonymousCredentialsProvider` (new) |

Environment variable names remain the same: `OSS_ACCESS_KEY_ID`, `OSS_ACCESS_KEY_SECRET`, `OSS_SESSION_TOKEN`.

### Custom credentials

V1 requires defining a new class that inherits `CredentialsProvider`. V2 adds `CredentialsProviderFunc`, which wraps a lambda or `std::function<Credentials()>`, so custom credential sources can be integrated without writing a class.

```cpp
// v1 -- must define a class
class MyCredentialsProvider : public AlibabaCloud::OSS::CredentialsProvider {
  public:
    AlibabaCloud::OSS::Credentials getCredentials() override {
        auto [ak, sk, token] = fetchFromSecretsManager();
        return AlibabaCloud::OSS::Credentials(ak, sk, token);
    }
};

auto provider = std::make_shared<MyCredentialsProvider>();
OssClient client(endpoint, provider, conf);
```

```cpp
// v2 -- use CredentialsProviderFunc with a lambda
conf.credentialsProvider = std::make_shared<oss::CredentialsProviderFunc>(
    []() -> oss::Credentials {
        auto [ak, sk, token] = fetchFromSecretsManager();
        return oss::Credentials(ak, sk, token);
    });
oss::OSSClient client(conf);
```

## HTTP transport

V1 only supports libcurl. V2 supports libcurl and WinHTTP (Windows only), and provides both synchronous and asynchronous HTTP transport interfaces.

| | V1 | V2 |
|:---|:---|:---|
| libcurl (sync) | Supported (only option) | `USE_CURL_TRANSPORT=ON` (default) |
| WinHTTP (sync) | -- | `USE_WINHTTP_TRANSPORT=ON` (Windows only) |
| libcurl async (curl_multi) | -- | Supported (`CurlMultiTransport`) |
| WinHTTP async | -- | Supported (`WinHttpAsyncTransport`) |
| Custom sync transport | Implement `HttpClient` interface, set `conf.httpClient` | Implement `HttpTransport` interface, set `conf.httpTransport` |
| Custom async transport | -- | Implement `AsyncHttpTransport` interface, set `conf.asyncHttpTransport` |
| CMake option | -- | `USE_CURL_TRANSPORT` / `USE_WINHTTP_TRANSPORT` |
| vcpkg feature | -- | `curl` (default) / `winhttp` |

```cpp
// v2 -- use default (libcurl)
auto conf = oss::ClientConfiguration::loadDefault();
oss::OSSClient client(conf);

// v2 -- custom transport
conf.httpTransport = myCustomTransport;
oss::OSSClient client(conf);

// v2 -- custom async transport (for OSSAsyncClient)
conf.asyncHttpTransport = myCustomAsyncTransport;
oss::OSSAsyncClient asyncClient(conf);
```

### Transport configuration

V1 puts all network parameters directly in `ClientConfiguration`. V2 separates them into two layers: common parameters remain in `ClientConfiguration` (automatically passed to the default transport), while transport-specific parameters are configured via `CurlTransportOptions` or `WinHttpTransportOptions` when creating a custom transport.

Common parameters (`ClientConfiguration`)

| V1 `ClientConfiguration` | V2 `ClientConfiguration` | Notes |
|:---|:---|:---|
| `connectTimeoutMs` | `connectTimeout` | Same unit (ms) |
| `requestTimeoutMs` | `readWriteTimeout` | Renamed |
| `verifySSL` | `insecureSkipVerify` | Inverted logic |
| `proxyScheme` + `proxyHost` + `proxyPort` | `proxyHost` | V2 uses a single URL string |
| `enabledRedirect` | `enabledRedirect` | Same |

Transport-specific parameters (`CurlTransportOptions`)

| V1 `ClientConfiguration` | V2 `CurlTransportOptions` | Notes |
|:---|:---|:---|
| `maxConnections` | `maxConnections` | Default: sync 16, async 100 |
| `caPath` | `caPath` | CA certificate directory (CURLOPT_CAPATH) |
| `caFile` | `caFile` | CA bundle file (CURLOPT_CAINFO) |
| `networkInterface` | `networkInterface` | Bind to network interface (CURLOPT_INTERFACE) |
| `proxyPort` | `proxyPort` | Proxy port (CURLOPT_PROXYPORT) |
| `proxyUserName` | `proxyUserName` | Proxy auth username |
| `proxyPassword` | `proxyPassword` | Proxy auth password |
| -- | `enableVerbose` | Enable curl verbose debug output |
| `httpInterceptor` | `requestInterceptor` | V1 uses `HttpInterceptor` interface; V2 uses a callback to set arbitrary curl options via `curl_easy_setopt` |

```cpp
// v1 -- all in ClientConfiguration
ClientConfiguration conf;
conf.maxConnections = 32;
conf.caPath = "/etc/ssl/certs";
conf.caFile = "/etc/ssl/certs/ca-certificates.crt";
conf.networkInterface = "eth0";
conf.proxyHost = "proxy.example.com";
conf.proxyPort = 8080;
conf.proxyUserName = "user";
conf.proxyPassword = "pass";
```

```cpp
// v2 -- create CurlTransportOptions and build transport
#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"

oss::CurlTransportOptions opts;
opts.maxConnections = 32;
opts.caPath = "/etc/ssl/certs";
opts.caFile = "/etc/ssl/certs/ca-certificates.crt";
opts.networkInterface = "eth0";
opts.proxyHost = "http://proxy.example.com";
opts.proxyPort = 8080;
opts.proxyUserName = "user";
opts.proxyPassword = "pass";
opts.enableVerbose = true;

auto conf = oss::ClientConfiguration::loadDefault();
conf.httpTransport = oss::CurlTransportFactory::createHttpTransport(opts);
oss::OSSClient client(conf);

// for OSSAsyncClient
conf.asyncHttpTransport = oss::CurlTransportFactory::createAsyncHttpTransport(opts);
oss::OSSAsyncClient asyncClient(conf);
```
