# Alibaba Cloud OSS SDK for C++ V2

[![GitHub version](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2.svg)](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2)

alibabacloud-oss-cpp-sdk-v2 是 OSS 在 C++ 编程语言下的第二版 SDK

## [README in English](README.md)

## 关于

> - 此 C++ SDK 基于[阿里云对象存储服务](http://www.aliyun.com/product/oss/)官方 API 构建。
> - 阿里云对象存储（Object Storage Service，简称 OSS），是阿里云对外提供的海量、安全、低成本、高可靠的云存储服务。
> - OSS 适合存放任意文件类型，适合各种网站、开发企业及开发者使用。
> - 使用此 SDK，用户可以方便地在任何应用、任何时间、任何地点上传、下载和管理数据。

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
find_package(alibabacloud-oss-v2 REQUIRED)

target_link_libraries(your_target PRIVATE alibabacloud::oss2)
```

## 快速使用

### 配置凭证

在使用 SDK 之前，您需要配置访问凭证。推荐使用环境变量方式：

```bash
export OSS_ACCESS_KEY_ID="your_access_key_id"
export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
export OSS_ENDPOINT="oss-cn-hangzhou.aliyuncs.com"
export OSS_REGION="cn-hangzhou"
```

#### 获取存储空间列表（List Buckets）

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <iostream>

using namespace alibabacloud::oss2;

int main() {
    // 初始化客户端配置
    auto conf = ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();

    // 创建 OSS 客户端
    OSSClient client(conf);

    // 列举存储空间
    auto outcome = client.listBuckets(models::ListBucketsRequest());
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListBuckets fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    for (const auto& bucket : result.getBuckets()) {
        std::cout << "存储空间: " << bucket.name
                  << ", 地域: " << bucket.location
                  << ", 存储类型: " << bucket.storageClass << std::endl;
    }
    return 0;
}
```

#### 获取文件列表（List Objects）

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <iostream>

using namespace alibabacloud::oss2;

int main() {
    auto conf = ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();
    OSSClient client(conf);

    auto outcome = client.listObjectsV2(
        models::ListObjectsV2Request()
            .setBucket("your-bucket-name")
    );
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectsV2 fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    for (const auto& object : result.getContents()) {
        std::cout << "文件: " << object.key
                  << ", 大小: " << object.size
                  << ", 最后修改时间: " << object.lastModified << std::endl;
    }
    return 0;
}
```

#### 上传文件（Put Object）

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include <iostream>

using namespace alibabacloud::oss2;

int main() {
    auto conf = ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();
    OSSClient client(conf);

    std::string content = "Hello, OSS!";
    auto body = RequestBody::FromString(content);

    auto outcome = client.putObject(
        models::PutObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key")
            .setBody(body)
    );
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObject fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "上传成功! statusCode: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    return 0;
}
```

#### 下载文件（Get Object）

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <iostream>
#include <sstream>

using namespace alibabacloud::oss2;

int main() {
    auto conf = ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();
    OSSClient client(conf);

    auto outcome = client.getObject(
        models::GetObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key")
    );
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetObject fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::stringstream buffer;
    buffer << result.getBody()->rdbuf();

    std::cout << "下载成功! 内容: " << buffer.str() << std::endl;
    return 0;
}
```

## 更多示例

请参看 `samples` 目录获取更多示例项目。

### 运行示例

> - 进入示例代码目录 `samples`。
> - 通过环境变量配置访问凭证：
>   ```bash
>   export OSS_ACCESS_KEY_ID="your_access_key_id"
>   export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
>   export OSS_REGION="cn-hangzhou"
>   ```
> - 在项目根目录下构建示例代码：
>   ```bash
>   mkdir build && cd build
>   cmake .. -DBUILD_SAMPLES=ON
>   cmake --build .
>   ```
> - 运行示例：
>   ```bash
>   ./samples/sample_ListBuckets --region cn-hangzhou
>   ```

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
