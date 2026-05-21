# Alibaba Cloud OSS SDK for C++ V2

[![GitHub version](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2.svg)](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2)

alibabacloud-oss-cpp-sdk-v2 是 OSS 在 C++ 编程语言下的第二版 SDK

## [README in English](README.md)

## 关于

> - 此 C++ SDK 基于[阿里云对象存储服务](http://www.aliyun.com/product/oss/)官方 API 构建。
> - 阿里云对象存储（Object Storage Service，简称 OSS），是阿里云对外提供的海量、安全、低成本、高可靠的云存储服务。
> - OSS 适合存放任意文件类型，适合各种网站、开发企业及开发者使用。
> - 使用此 SDK，用户可以方便地在任何应用、任何时间、任何地点上传、下载和管理数据。

## 特性

- **同步与异步** -- `OSSClient` 同步调用；`OSSAsyncClient` 全异步操作；`OSSClient::asyncCall()` 在执行器上异步运行同步操作
- **自动重试** -- 可配置的指数退避重试策略，应对瞬时故障
- **分页器** -- `makePaginator()` 自动翻页迭代列举操作
- **预签名 URL** -- 为 PutObject、GetObject、HeadObject、UploadPart 生成预签名 URL
- **请求取消** -- `CancellationToken` 取消进行中的请求；`disableRequest()` 客户端级别批量取消
- **进度回调** -- 跟踪上传/下载进度
- **灵活的请求体** -- 从 String、File、Stream、Memory buffer 构造
- **多种凭证提供者** -- 环境变量、静态凭证、STS、ECS RAM Role、自定义提供者
- **可定制传输层** -- 自定义 Curl 或 WinHTTP 配置
- **`std::expected` 支持** -- 可选的 C++23 模式，支持一元错误处理

## 运行环境

> - C++17 及以上版本（`std::expected` 模式需要 C++23）
> - CMake 3.15 及以上版本
> - 支持平台：Linux、macOS、Windows、Android

## 安装方法

### 通过源码安装

当您从 GitHub 下载代码后，可以使用 CMake 进行构建和安装：

```bash
# 克隆仓库
git clone https://github.com/aliyun/alibabacloud-oss-cpp-sdk-v2.git
cd alibabacloud-oss-cpp-sdk-v2

# 创建构建目录
mkdir build && cd build

# 使用 CMake 配置
cmake ..

# 构建项目
cmake --build .

# 安装（可选）
sudo cmake --install .
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_SHARED_LIBS` | `OFF` | 构建动态库而非静态库 |
| `BUILD_TESTS` | `OFF` | 构建单元测试 |
| `BUILD_SAMPLES` | `OFF` | 构建示例程序 |
| `ENABLE_RTTI` | `ON` | 启用/禁用 RTTI 信息 |
| `USE_SYSTEM_CURL` | `OFF` | 使用系统已安装的 libcurl |
| `USE_SYSTEM_OPENSSL` | `OFF` | 使用系统已安装的 OpenSSL |
| `USE_SYSTEM_MBEDTLS` | `OFF` | 使用系统已安装的 mbedTLS |
| `USE_STD_EXPECTED` | `OFF` | 使用 `std::expected` 替代自定义 `Outcome`（需要 C++23） |
| `ENABLE_COVERAGE` | `OFF` | 生成代码覆盖率报告 |
| `ENABLE_CPPCHECK` | `OFF` | 启用 Cppcheck 静态分析 |
| `ENABLE_SANITIZER` | `OFF` | 启用 Sanitizer 检测 |

启用 `std::expected` 模式（C++23）：

```bash
cmake -B build -DCMAKE_CXX_STANDARD=23 -DUSE_STD_EXPECTED=ON
cmake --build build
```

### 在您的项目中使用 CMake

在您的 `CMakeLists.txt` 中添加以下内容：

```cmake
find_package(alibabacloud_oss_v2 REQUIRED)

target_link_libraries(your_target PRIVATE alibabacloud_oss_v2::oss)
```

## 快速使用

### 配置凭证

在使用 SDK 之前，您需要配置访问凭证。推荐使用环境变量方式：

```bash
export OSS_ACCESS_KEY_ID="your_access_key_id"
export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
```

### 快速开始

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <iostream>
#include <sstream>

namespace oss = alibabacloud::oss2;

int main() {
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    oss::OSSClient client(conf);

    // 上传
    auto putOutcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key")
            .setBody(oss::RequestBody::FromString("Hello, OSS!")));
    if (!putOutcome.has_value()) {
        std::cerr << "PutObject fail, code: " << putOutcome.error().getCode()
                  << ", message: " << putOutcome.error().getMessage() << std::endl;
        return 1;
    }
    std::cout << "Upload successful, ETag: " << putOutcome.value().getETag() << std::endl;

    // 下载
    auto getOutcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key"));
    if (!getOutcome.has_value()) {
        std::cerr << "GetObject fail, code: " << getOutcome.error().getCode()
                  << ", message: " << getOutcome.error().getMessage() << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << getOutcome.value().getBody()->rdbuf();
    std::cout << "Download successful, content: " << buffer.str() << std::endl;

    return 0;
}
```

## 更多示例

更多示例请参阅 [`samples`](samples/INDEX.md) 目录，包括同步/异步 API 用法、分页器，以及场景示例（进度回调、传输层定制、重试策略、请求取消、凭证提供者等）。

## 错误处理

SDK 使用 `Outcome<Result, Error>`，接口兼容 `std::expected`：

```cpp
auto outcome = client.putObject(request);

if (!outcome.has_value()) {
    auto& err = outcome.error();
    std::cerr << "错误码: " << err.getCode() << std::endl;
    std::cerr << "错误信息: " << err.getMessage() << std::endl;
    std::cerr << "请求 ID: " << err.getRequestId() << std::endl;
    std::cerr << "操作名称: " << err.getOpName() << std::endl;
    std::cerr << "请求方法: " << err.getMethod() << std::endl;
}
```

使用 `-DUSE_STD_EXPECTED=ON`（C++23）构建时，`Outcome` 将成为 `std::expected` 的类型别名，支持 `.and_then()`、`.transform()`、`.or_else()` 等一元操作。

为兼容从 [aliyun-oss-cpp-sdk](https://github.com/aliyun/aliyun-oss-cpp-sdk) 迁移的用户，SDK 同时保留了旧版接口（`isSuccess()` / `getResult()` / `getError()`）。注意：旧版接口在 `std::expected` 模式下不可用。

```cpp
auto outcome = client.putObject(request);

if (!outcome.isSuccess()) {
    auto& error = outcome.getError();
    std::cerr << "错误码: " << error.getCode() << std::endl;
    std::cerr << "错误信息: " << error.getMessage() << std::endl;
}

auto& result = outcome.getResult();
```

## 线程安全

`OSSClient` 实例是线程安全的，可以在多个线程之间共享。但是请求和结果对象不是线程安全的，不应在线程之间共享。

## 许可协议

> - Apache-2.0，请参阅 [许可文件](LICENSE)
