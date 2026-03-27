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

> - C++17 及以上版本
> - CMake 3.10 及以上版本
> - 支持平台：Linux、macOS、Windows

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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "ListBuckets fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "ListObjectsV2 fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "PutObject fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "GetObject fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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

SDK 通过 `Outcome` 模式提供详细的错误信息：

```cpp
auto outcome = client.putObject(request);

if (!outcome.isSuccess()) {
    auto& error = outcome.getError();
    std::cerr << "错误码: " << error.getCode() << std::endl;
    std::cerr << "错误信息: " << error.getMessage() << std::endl;
    std::cerr << "请求 ID: " << error.getRequestId() << std::endl;
    std::cerr << "操作名称: " << error.getOpName() << std::endl;
    std::cerr << "请求方法: " << error.getMethod() << std::endl;
}
```

## 线程安全

`OSSClient` 实例是线程安全的，可以在多个线程之间共享。但是请求和结果对象不是线程安全的，不应在线程之间共享。

## 许可协议

> - Apache-2.0，请参阅 [许可文件](LICENSE)
