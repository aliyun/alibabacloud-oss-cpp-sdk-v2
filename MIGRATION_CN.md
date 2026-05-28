# 迁移指南

[English](MIGRATION.md)

本部分介绍如何从V1版本 ([aliyun-oss-cpp-sdk](https://github.com/aliyun/aliyun-oss-cpp-sdk)) 迁移到 V2 版本。

## 最低 C++ 版本

V2 版本 要求 C++17 或更高版本。V1 版本要求 C++11。

## 头文件路径和命名空间

V2 版本使用新的代码仓库，同时对头文件路径进行了调整。命名空间从 `AlibabaCloud::OSS` 修改为 `alibabacloud::oss2`。

| 模块 | V1 | V2 |
|:-----|:---|:---|
| 核心 | `alibabacloud/oss/OssClient.h` | `alibabacloud/oss2/OSSClient.h` |
| 核心（异步） | -- | `alibabacloud/oss2/OSSAsyncClient.h` |
| 配置 | `alibabacloud/oss/client/ClientConfiguration.h` | `alibabacloud/oss2/ClientConfiguration.h` |
| 凭证 | `alibabacloud/oss/auth/CredentialsProvider.h` | `alibabacloud/oss2/credentials/CredentialsProvider.h` |
| 模型 | `alibabacloud/oss/model/<Name>.h` | `alibabacloud/oss2/models/<Category>.h` |
| 前置声明 | `alibabacloud/oss/OssFwd.h` | `alibabacloud/oss2/OSSFwd.h` |
| 加密 | `alibabacloud/oss/encryption/*.h` | `alibabacloud/oss2/crypto/*.h` |

示例

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

## SDK 初始化

V1 版本需要在使用前调用 `InitializeSdk()`, 退出时调用 `ShutdownSdk()`。V2 版本移除了此要求。

```cpp
// v1
AlibabaCloud::OSS::InitializeSdk();
// ... 使用 SDK ...
AlibabaCloud::OSS::ShutdownSdk();
```

```cpp
// v2
// 不需要初始化和销毁
```

## 配置

V2 版本将所有配置统一到 `ClientConfiguration` 中。凭证和访问域名不再作为构造函数参数传递，而是通过 `ClientConfiguration` 字段设置。

V2 版本的可选字段使用 `std::optional`，未设置的字段会使用合理的默认值。

示例

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
conf.connectTimeout = 20000;       // 毫秒
conf.readWriteTimeout = 60000;     // 毫秒
conf.insecureSkipVerify = true;

oss::OSSClient client(conf);
```

配置参数对比

| V1 | V2 | 说明 |
|:---|:---|:-----|
| `endpoint` (构造函数) | `ClientConfiguration::endpoint` | V2 可从 `region` 自动推导 |
| `accessKeyId`/`accessKeySecret` (构造函数) | `ClientConfiguration::credentialsProvider` | 统一使用凭证提供者 |
| -- | `ClientConfiguration::region` | V2 新增 |
| `connectTimeoutMs` | `connectTimeout` | 单位相同（毫秒） |
| `requestTimeoutMs` | `readWriteTimeout` | 重命名 |
| `verifySSL` | `insecureSkipVerify` | 逻辑取反：v1 true=校验, v2 true=跳过 |
| `isCname` | `useCName` | 相同 |
| `isPathStyle` | `usePathStyle` | 相同 |
| `enableCrc64` | `disableUploadCRC64Check` / `disableDownloadCRC64Check` | 逻辑取反；V2 默认启用 CRC64，可通过这两个字段分别关闭上传和下载的校验 |
| `enableDateSkewAdjustment` | `disableClockSkewCorrection` | 逻辑取反 |
| `signatureVersion` | `signatureVersion` | V1 默认 "v1"，V2 默认 "v4"（需要设置 `region`） |
| `retryStrategy` | `retryer` / `retryMaxAttempts` | V2 内置重试和退避 |
| `proxyHost`/`proxyPort`/... | `proxyHost` | V2 使用单个 URL 字符串 |
| `httpClient` | `httpTransport` | 接口不同 |

## 创建 Client

V2 版本修改了构造函数，只接受 `ClientConfiguration`。访问域名和凭证通过配置对象设置。

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

## 调用 API 操作

V2 版本采用统一的模式：方法名使用 `camelCase` 风格，每个操作接受一个 `<OperationName>Request` 参数并返回 `<OperationName>Outcome`（即 `Outcome<Result, Error>`）。V1 中的便捷重载（如 `PutObject(bucket, key, content)`）被移除。

V2 的请求对象使用 `set*` 链式方法。

```
OutcomeType operationName(const models::OperationNameRequest& request, const OperationOptions* options = nullptr)
```

示例

```cpp
// v1
auto outcome = client.PutObject("mybucket", "mykey", 
    std::make_shared<std::stringstream>("hello"));
if (outcome.isSuccess()) {
    // 使用 outcome.result()
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
        .setBody(oss::RequestBody::FromString("hello")));
if (outcome.has_value()) {
    // 使用 outcome.value() 或 *outcome
} else {
    auto& err = outcome.error();
    std::cerr << err.getCode() << ": " << err.getMessage() << std::endl;
}
```

方法名变更示例

大部分方法仅从 PascalCase 改为 camelCase，少数方法同时进行了重命名：

| V1 | V2 | 说明 |
|:---|:---|:-----|
| `PutObject()` | `putObject()` | 仅大小写 |
| `GetObject()` | `getObject()` | 仅大小写 |
| `HeadObject()` | `headObject()` | 仅大小写 |
| `CreateBucket()` | `putBucket()` | 重命名 |
| `DeleteObjects()` | `deleteMultipleObjects()` | 重命名 |
| `SetObjectAcl()` | `putObjectAcl()` | 重命名 |
| `CreateSymlink()` | `putSymlink()` | 重命名 |

## 错误处理

V1 版本使用 `Outcome` 类型，通过 `isSuccess()`、`result()` 和 `error()` 方法访问。V2 版本使用 `Outcome<Result, Error>`，接口兼容 `std::expected`。

| | V1 | V2 |
|:---|:---|:---|
| 返回类型 | `Outcome` | `Outcome<Result, Error>` |
| 判断成功 | `isSuccess()` | `has_value()` |
| 获取结果 | `result()` | `value()` 或 `operator*` |
| 获取错误 | `error()` | `error()` |
| 错误码 | `error().Code()` | `error().getCode()` |
| 错误信息 | `error().Message()` | `error().getMessage()` |
| 请求 ID | `error().RequestId()` | `error().getRequestId()` |
| 错误类别 | -- | `error().getEC()`（`std::error_code`） |
| 请求地址 | -- | `error().getRequestTarget()` |

```cpp
// v1
auto outcome = client.GetObject(request);
if (outcome.isSuccess()) {
    auto& result = outcome.result();
    // 使用 result.Content()
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
// 使用 result.getBody()
```

使用 `-DUSE_STD_EXPECTED=ON`（C++23）构建时，`Outcome` 将成为 `std::expected` 的类型别名，支持 `.and_then()`、`.transform()`、`.or_else()` 等一元操作。

为兼容 V1 迁移，V2 同时保留了旧版接口（`isSuccess()` / `getResult()` / `getError()`）。注意：旧版接口在 `std::expected` 模式下不可用。

## 请求体

V1 版本所有操作统一使用 `std::shared_ptr<std::iostream>` 作为上传数据体。V2 版本使用 `RequestBody` 命名空间提供四种工厂方法，每种对应不同的内容类型，可根据数据来源选择最合适的方式：

| 工厂方法 | 底层类型 | 所有权 | 说明 |
|:---------|:---------|:-------|:-----|
| `RequestBody::FromString(data)` | `StringContent` | 拥有（拷贝/移动） | 拷贝或移动 `std::string` 到请求体中，适用于所有场景 |
| `RequestBody::FromFile(path)` | `FileContent` | 拥有（路径） | 从文件路径读取数据，重试时自动重新打开文件 |
| `RequestBody::FromStream(stream)` | `StreamContent` | 共享（`shared_ptr`） | 包装 `std::shared_ptr<std::istream>`，传入 null 时返回 `EmptyContent` |
| `RequestBody::FromMemory(data, len)` | `MemoryContent` | 非拥有（零拷贝） | 直接引用已有内存，不做拷贝。调用方必须确保数据在请求完成前有效 |

```cpp
// v1
auto content = std::make_shared<std::stringstream>("data");
auto outcome = client.PutObject("bucket", "key", content);

// 从文件上传
auto outcome2 = client.PutObject("bucket", "key", "/path/to/file");
```

```cpp
// v2 -- FromString: 拥有数据的拷贝
auto outcome = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::FromString("hello world")));

// v2 -- FromFile: 从文件路径读取，重试时自动重新打开
auto outcome2 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::FromFile("/path/to/data.bin")));

// v2 -- FromStream: 包装共享的 istream
auto ifs = std::make_shared<std::ifstream>("data.bin", std::ios::binary);
auto outcome3 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::FromStream(ifs)));

// v2 -- FromMemory: 零拷贝，非拥有引用
const char* buf = getBuffer();
size_t bufLen = getBufferSize();
auto outcome4 = client.putObject(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setBody(oss::RequestBody::FromMemory(buf, bufLen)));
```

V2 版本还提供了 `putObjectFromFile()` 便捷方法，可直接传入文件路径：

```cpp
auto outcome = client.putObjectFromFile(
    oss::models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    "/path/to/file");
```

## 响应体

V1 和 V2 默认都通过 `std::shared_ptr<std::iostream>` 返回下载数据。两者也都支持自定义响应工厂来控制数据接收方式，但接口有所不同：

| | V1 | V2 |
|:---|:---|:---|
| 获取响应体 | `result.Content()` | `result.getBody()` |
| 响应体类型 | `std::shared_ptr<std::iostream>` | `std::shared_ptr<std::iostream>` |
| 自定义工厂 | `request.setResponseStreamFactory(factory)` | `request.setSinkFactory(factory)` |
| 工厂类型 | `IOStreamFactory` = `std::function<std::shared_ptr<std::iostream>()>` | `SinkFactory`，含 `std::function<std::shared_ptr<ByteWriter>(int64_t size, const HeaderCollection& headers)>` |
| 工厂参数 | 无 | 内容长度 + 响应头 |
| 重试支持 | -- | `factory.isOneShot` 标志 |

```cpp
// v1 -- 从 iostream 读取
auto outcome = client.GetObject(request);
if (outcome.isSuccess()) {
    auto& content = outcome.result().Content();
    std::string data(std::istreambuf_iterator<char>(*content), {});
}

// v1 -- 自定义响应流工厂
request.setResponseStreamFactory([&]() {
    return std::make_shared<std::fstream>("/path/to/file",
        std::ios::out | std::ios::binary);
});
```

```cpp
// v2 -- 从 iostream 读取
auto outcome = client.getObject(request);
if (outcome.has_value()) {
    auto& body = outcome->getBody();
    std::string data(std::istreambuf_iterator<char>(*body), {});
}

// v2 -- 自定义接收器（零拷贝下载到用户提供的缓冲区）
std::vector<uint8_t> buf(4 * 1024 * 1024);
oss::SinkFactory factory;
factory.supplier = [&buf](std::int64_t, const oss::HeaderCollection&)
    -> std::shared_ptr<oss::ByteWriter> {
    return std::make_shared<oss::MemoryWriter>(buf.data(), buf.size());
};
factory.isOneShot = false;  // 支持重试

auto outcome2 = client.getObject(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key")
        .setSinkFactory(factory));
```

## 请求取消

V1 版本仅支持客户端级别的取消。V2 版本新增了基于 `CancellationToken` 的单请求取消。

| 层级 | V1 | V2 |
|:-----|:---|:---|
| 单个请求 | -- | 通过 `OperationOptions` 设置 `CancellationToken` |
| 客户端级别 | `client.DisableRequest()` / `client.EnableRequest()` | `client.disableRequest()` / `client.enableRequest()` |

单请求取消使用 `CancellationTokenSource` 创建 token，支持立即取消或基于截止时间的取消。

```cpp
// v2 -- 从另一个线程取消单个请求
auto cts = oss::CancellationTokenSource::create();

oss::OperationOptions opts;
opts.cancellationToken = cts->getToken();

// 在另一个线程中发起请求
auto future = std::async([&]() {
    return client.getObject(request, &opts);
});

// 从当前线程取消
cts->cancel();

// 或设置基于截止时间的取消
auto cts2 = oss::CancellationTokenSource::create();
cts2->cancelAfter(std::chrono::seconds(30));
opts.cancellationToken = cts2->getToken();
```

客户端级别取消会禁用该客户端实例上的所有请求。新请求会立即失败，直到重新启用。注意 `disableRequest()` 仅阻止新的 HTTP 调用发出；如果同时使用了 per-request 的 `CancellationToken`，需要显式取消这些 token 才能打断已经在进行中的请求。

```cpp
// v2 -- 取消客户端上的所有请求（包括使用了 CancellationToken 的请求）
client.disableRequest();       // 阻止新请求并中止等待中的请求
cts->cancel();                 // 同时取消持有该 token 的进行中的请求

// ... 之后重新启用
client.enableRequest();
```

## 预签名 URL

V1 版本使用 `GeneratePresignedUrl()` 返回 URL 字符串。V2 版本使用 `presign()` 的类型化请求重载，返回 `PresignResult`，包含 URL、HTTP 方法、过期时间和签名请求头。

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

## 异步操作

V1 版本在 `OssClient` 上为每个操作提供独立的 `*Async()` 和 `*Callable()` 方法。V2 版本提供两种方式：

| | V1 `OssClient` | V2 `OSSClient` | V2 `OSSAsyncClient` |
|:---|:---|:---|:---|
| 头文件 | `alibabacloud/oss/OssClient.h` | `alibabacloud/oss2/OSSClient.h` | `alibabacloud/oss2/OSSAsyncClient.h` |
| HTTP 传输 | 同步（线程池包装） | 同步（线程池包装） | 原生异步 HTTP |
| 需要 `Executor` | 否（内置） | 是（`conf.executor`） | 否 |
| 基于 Future | `GetObjectCallable()` | `asyncCall(request)` | `asyncCall(request)` |
| 基于回调 | `GetObjectAsync(req, cb)` | `asyncCallback(req, cb)` | `getObjectAsync(req, cb)` |
| 方法风格 | 每个操作独立 `*Async()`/`*Callable()` | 泛型模板，适用于所有操作 | 每个操作独立 `*Async()` + 泛型 `asyncCall()` |

### OSSClient 异步（同步客户端 + 线程池）

V2 `OSSClient` 使用泛型模板方法 `asyncCall()` 和 `asyncCallback()`，替代 V1 中每个操作独立的异步方法。

```cpp
// v1 - 每个操作独立的回调方法
client.GetObjectAsync(request, 
    [](const OssClient*, const GetObjectRequest& req, const GetObjectOutcome& outcome, 
       const std::shared_ptr<const AsyncCallerContext>&) {
        if (outcome.isSuccess()) { /* ... */ }
    });

// v1 - 每个操作独立的 future 方法
auto task = client.GetObjectCallable(request);
auto outcome = task.get();
```

```cpp
// v2 OSSClient - 需要配置 executor
conf.executor = std::make_shared<oss::ThreadPoolExecutor>(4);
oss::OSSClient client(conf);

// 基于 future（泛型模板）
auto future = client.asyncCall(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"));
auto outcome = future.get();

// 基于回调（泛型模板）
client.asyncCallback(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    [](const oss::OSSClient*, const oss::models::GetObjectRequest&, 
       const oss::GetObjectOutcome& outcome) {
        if (outcome.has_value()) { /* ... */ }
    });
```

### OSSAsyncClient（原生异步 HTTP）

V2 版本还提供了 `OSSAsyncClient`，支持基于原生异步 HTTP 传输的完全异步操作，无需配置 `Executor`。

```cpp
// v2 OSSAsyncClient
oss::OSSAsyncClient asyncClient(conf);

// 每个操作独立的回调方法
asyncClient.getObjectAsync(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"),
    [](oss::GetObjectOutcome outcome) {
        if (outcome.has_value()) { /* ... */ }
    });

// 基于 future（泛型模板）
auto future = asyncClient.asyncCall(
    oss::models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("key"));
auto outcome = future.get();
```

## 分页器

V1 版本需要手动使用标记(marker)进行分页。V2 版本提供 `makePaginator()` 自动分页。

```cpp
// v1 - 手动分页
std::string marker;
do {
    ListObjectsRequest request("mybucket");
    if (!marker.empty()) request.setMarker(marker);
    auto outcome = client.ListObjects(request);
    if (!outcome.isSuccess()) break;
    for (auto& obj : outcome.result().ObjectSummarys()) {
        // 处理 obj
    }
    if (outcome.result().IsTruncated()) {
        marker = outcome.result().NextMarker();
    } else {
        break;
    }
} while (true);
```

```cpp
// v2 - 自动分页
auto paginator = oss::makePaginator(client,
    oss::models::ListObjectsRequest()
        .setBucket("mybucket")
        .setMaxKeys(100));
while (paginator.hasNext()) {
    auto outcome = paginator.nextPage();
    if (!outcome.has_value()) break;
    for (auto& obj : outcome->getContents()) {
        // 处理 obj
    }
}
```

支持的分页器：`ListBucketsRequest`、`ListObjectsRequest`、`ListObjectsV2Request`、`ListObjectVersionsRequest`、`ListMultipartUploadsRequest`、`ListPartsRequest`。

## 断点续传

V1 版本提供 `ResumableUploadObject()`、`ResumableDownloadObject()` 和 `ResumableCopyObject()`。V2 版本未来将提供 `Uploader`、`Downloader` 和 `Copier` 来支持该能力。当前 `getObjectToFile()` 支持网络失败后通过 Range 请求自动恢复下载。

| 场景 | V1 | V2 |
|:-----|:---|:---|
| 上传文件 | `PutObject(bucket, key, filePath)` | `putObjectFromFile(request, filePath)` |
| 下载到文件 | `GetObject(bucket, key, filePath)` | `getObjectToFile(request, filePath)` |
| 上传大文件（断点续传） | `ResumableUploadObject(request)` | `Uploader`（TBD） |
| 下载大文件（断点续传） | `ResumableDownloadObject(request)` | `Downloader`（TBD）；当前 `getObjectToFile()` 支持通过 Range 请求自动恢复 |

## 便捷方法

V1 版本提供了大量的便捷重载（如 `GetObject(bucket, key)`、`PutObject(bucket, key, content)`）。V2 版本移除了这些重载，每个操作只保留一个方法，使用链式请求对象。V2 版本同时提供了一些扩展方法：

| V1 | V2 |
|:---|:---|
| `PutObject(bucket, key, filePath)` | `putObjectFromFile(request, filePath)` |
| `GetObject(bucket, key, filePath)` | `getObjectToFile(request, filePath)` |
| `DoesBucketExist(bucket)` | `isBucketExist(bucket)` |
| `DoesObjectExist(bucket, key)` | `isObjectExist(bucket, key)` |

## 客户端加密

V1 版本使用继承自 `OssClient` 的 `OssEncryptionClient`，配合 `ContentCryptoMaterial` 和 `EncryptionMaterials`。V2 版本使用 `OSSEncryptionClient`，采用简化的 `MasterCipher` 接口和 `EncryptionConfiguration`。

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
        .setBody(oss::RequestBody::FromString("data")));
```

V2 版本的 `OSSEncryptionClient` 支持：`putObject`、`getObject`、`headObject`、`getObjectMeta`、`initiateMultipartUpload`、`uploadPart`、`completeMultipartUpload`、`abortMultipartUpload`、`listParts`，以及 `unwrap()` 获取底层 `OSSClient` 访问非加密操作。

## 重试

V1 和 V2 均默认开启自动重试（最大尝试 3 次）。V2 使用 `FullJitterBackoff` 实现带抖动的指数退避。

| | V1 | V2 |
|:---|:---|:---|
| 默认最大尝试次数 | 3 | 3 |
| 退避策略 | 基于 `scaleFactor` | `FullJitterBackoff`（带抖动的指数退避） |
| 配置方式 | `ClientConfiguration::retryStrategy` | `ClientConfiguration::retryer` / `retryMaxAttempts` |
| 单请求覆盖 | -- | `OperationOptions::retryMaxAttempts` |
| 接口 | `RetryStrategy`（继承 `shouldRetry` + `calcDelayTimeMs`） | `Retryer` |

## CRC-64

CRC-64 用于在上传和下载过程中校验数据完整性。V1 和 V2 均默认对上传操作启用 CRC-64 校验。

V1 的 `getObject()` 流式下载同样启用了 CRC-64 校验 —— HTTP 客户端在接收数据时计算 CRC-64，并与服务器返回的 `x-oss-hash-crc64ecma` 响应头进行比对。V2 的 `getObject()` 流式读取**不**执行 CRC-64 校验；如需下载时校验 CRC-64，请使用 `getObjectToFile()`，或通过 `SinkFactory` + `CRC64WriteObserver` 手动校验。

| Operation | V1 | V2 |
|:----------|:---|:---|
| `putObject` | CRC-64 enabled | CRC-64 enabled |
| `appendObject` | CRC-64 enabled | CRC-64 enabled |
| `uploadPart` | CRC-64 enabled | CRC-64 enabled |
| `getObject` (streaming) | CRC-64 enabled | No CRC-64 |
| `getObjectToFile` / `ResumableDownloadObject` | CRC-64 enabled | CRC-64 enabled |
| Disable upload CRC | `enableCrc64 = false` | `disableUploadCRC64Check = true` |
| Disable download CRC | `enableCrc64 = false` | `disableDownloadCRC64Check = true` |

V2 中对 `getObject()` 流式下载手动校验 CRC-64，可使用 `SinkFactory` + `CRC64WriteObserver`：

```cpp
// v2 -- getObject 流式下载手动 CRC-64 校验
auto crc = std::make_shared<oss::CRC64WriteObserver>();

oss::SinkFactory factory;
factory.isOneShot = false;
factory.supplier = [&crc](std::int64_t, const oss::HeaderCollection&)
    -> std::shared_ptr<oss::ByteWriter> {
    // retry 时重置 CRC，从头重新计算
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
        // CRC 不匹配，数据可能已损坏
    }
}
```

## 凭证

V2 版本提供了等效的凭证提供者，类名有所调整：

| V1 | V2 |
|:---|:---|
| `SimpleCredentialsProvider` | `StaticCredentialsProvider` |
| `EnvironmentVariableCredentialsProvider` | `EnvironmentVariableCredentialsProvider` |
| `CredentialsProvider`（接口） | `CredentialsProvider`（接口） |
| -- | `CredentialsProviderFunc`（新增） |
| -- | `AnonymousCredentialsProvider`（新增） |

环境变量名不变：`OSS_ACCESS_KEY_ID`、`OSS_ACCESS_KEY_SECRET`、`OSS_SESSION_TOKEN`。

### 自定义凭证

V1 版本需要定义一个继承 `CredentialsProvider` 的新类。V2 版本新增了 `CredentialsProviderFunc`，可以直接包装 lambda 或 `std::function<Credentials()>`，无需编写类即可接入自定义凭证源。

```cpp
// v1 -- 必须定义一个类
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
// v2 -- 使用 CredentialsProviderFunc 和 lambda
conf.credentialsProvider = std::make_shared<oss::CredentialsProviderFunc>(
    []() -> oss::Credentials {
        auto [ak, sk, token] = fetchFromSecretsManager();
        return oss::Credentials(ak, sk, token);
    });
oss::OSSClient client(conf);
```

## HTTP 传输

V1 版本仅支持 libcurl。V2 版本支持 libcurl 和 WinHTTP（仅 Windows），并提供同步和异步两种 HTTP 传输接口。

| | V1 | V2 |
|:---|:---|:---|
| libcurl（同步） | 支持（唯一选项） | `USE_CURL_TRANSPORT=ON`（默认） |
| WinHTTP（同步） | -- | `USE_WINHTTP_TRANSPORT=ON`（仅 Windows） |
| libcurl 异步（curl_multi） | -- | 支持（`CurlMultiTransport`） |
| WinHTTP 异步 | -- | 支持（`WinHttpAsyncTransport`） |
| 自定义同步传输 | 实现 `HttpClient` 接口，设置 `conf.httpClient` | 实现 `HttpTransport` 接口，设置 `conf.httpTransport` |
| 自定义异步传输 | -- | 实现 `AsyncHttpTransport` 接口，设置 `conf.asyncHttpTransport` |
| CMake 选项 | -- | `USE_CURL_TRANSPORT` / `USE_WINHTTP_TRANSPORT` |
| vcpkg 特性 | -- | `curl`（默认）/ `winhttp` |

```cpp
// v2 -- 使用默认（libcurl）
auto conf = oss::ClientConfiguration::loadDefault();
oss::OSSClient client(conf);

// v2 -- 自定义传输
conf.httpTransport = myCustomTransport;
oss::OSSClient client(conf);

// v2 -- 自定义异步传输（用于 OSSAsyncClient）
conf.asyncHttpTransport = myCustomAsyncTransport;
oss::OSSAsyncClient asyncClient(conf);
```

### 传输层配置参数

V1 版本将所有网络参数直接放在 `ClientConfiguration` 中。V2 版本将其分为两层：通用参数仍在 `ClientConfiguration` 中（自动传递给默认传输层），传输层特有的参数通过 `CurlTransportOptions` 或 `WinHttpTransportOptions` 在创建自定义传输时配置。

通用参数（`ClientConfiguration`）

| V1 `ClientConfiguration` | V2 `ClientConfiguration` | 说明 |
|:---|:---|:---|
| `connectTimeoutMs` | `connectTimeout` | 单位相同（毫秒） |
| `requestTimeoutMs` | `readWriteTimeout` | 重命名 |
| `verifySSL` | `insecureSkipVerify` | 逻辑取反 |
| `proxyScheme` + `proxyHost` + `proxyPort` | `proxyHost` | V2 使用单个 URL 字符串 |
| `enabledRedirect` | `enabledRedirect` | 相同 |

传输层特有参数（`CurlTransportOptions`）

| V1 `ClientConfiguration` | V2 `CurlTransportOptions` | 说明 |
|:---|:---|:---|
| `maxConnections` | `maxConnections` | 默认值：同步 16，异步 100 |
| `caPath` | `caPath` | CA 证书目录（CURLOPT_CAPATH） |
| `caFile` | `caFile` | CA 证书包文件（CURLOPT_CAINFO） |
| `networkInterface` | `networkInterface` | 绑定网络接口（CURLOPT_INTERFACE） |
| `proxyPort` | `proxyPort` | 代理端口（CURLOPT_PROXYPORT） |
| `proxyUserName` | `proxyUserName` | 代理认证用户名 |
| `proxyPassword` | `proxyPassword` | 代理认证密码 |
| -- | `enableVerbose` | 开启 curl 详细调试输出 |
| `httpInterceptor` | `requestInterceptor` | V1 使用 `HttpInterceptor` 接口；V2 使用回调函数，可通过 `curl_easy_setopt` 设置任意 curl 选项 |

```cpp
// v1 -- 全部在 ClientConfiguration 中
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
// v2 -- 创建 CurlTransportOptions 并构建传输层
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

// 用于 OSSAsyncClient
conf.asyncHttpTransport = oss::CurlTransportFactory::createAsyncHttpTransport(opts);
oss::OSSAsyncClient asyncClient(conf);
```
