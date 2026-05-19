# Alibaba Cloud OSS SDK for C++ V2

[![GitHub version](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2.svg)](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2)

alibabacloud-oss-cpp-sdk-v2 is the developer preview for the v2 of the OSS SDK for the C++ programming language

## [中文文档](README_CN.md)

## About

> - This C++ SDK is based on the official APIs of [Alibaba Cloud OSS](http://www.aliyun.com/product/oss/).
> - Alibaba Cloud Object Storage Service (OSS) is a cloud storage service provided by Alibaba Cloud, featuring massive capacity, security, a low cost, and high reliability.
> - The OSS can store any type of files and therefore applies to various websites, development enterprises and developers.
> - With this SDK, you can upload, download and manage data on any app anytime and anywhere conveniently.

## Running Environment

> - C++17 or later (C++23 required for `std::expected` mode)
> - CMake 3.15 or later
> - Supported platforms: Linux, macOS, Windows, Android

## Installing

### Install from the source code

Once you check out the code from GitHub, you can build it using CMake. Use the following commands to build:

```bash
# Clone the repository
git clone https://github.com/aliyun/alibabacloud-oss-cpp-sdk-v2.git
cd alibabacloud-oss-cpp-sdk-v2

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Install (optional)
sudo cmake --install .
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `OFF` | Build shared libraries instead of static |
| `BUILD_TESTS` | `OFF` | Build unit tests |
| `BUILD_SAMPLES` | `OFF` | Build sample programs |
| `ENABLE_RTTI` | `ON` | Enable/disable building code with RTTI information |
| `USE_SYSTEM_CURL` | `OFF` | Use system-installed libcurl |
| `USE_SYSTEM_OPENSSL` | `OFF` | Use system-installed OpenSSL |
| `USE_SYSTEM_MBEDTLS` | `OFF` | Use system-installed mbedTLS |
| `USE_STD_EXPECTED` | `OFF` | Use `std::expected` instead of custom `Outcome` (requires C++23) |
| `ENABLE_COVERAGE` | `OFF` | Generate coverage reports |
| `ENABLE_CPPCHECK` | `OFF` | Enable Cppcheck static analysis |
| `ENABLE_SANITIZER` | `OFF` | Enable sanitizers |

To enable `std::expected` mode (C++23):

```bash
cmake -B build -DCMAKE_CXX_STANDARD=23 -DUSE_STD_EXPECTED=ON
cmake --build build
```

### Using CMake in your project

Add the following to your `CMakeLists.txt`:

```cmake
find_package(alibabacloud-oss-v2 REQUIRED)

target_link_libraries(your_target PRIVATE alibabacloud::oss2)
```

## Getting Started

### Configuration

Before using the SDK, you need to configure your credentials. The recommended way is to use environment variables:

```bash
export OSS_ACCESS_KEY_ID="your_access_key_id"
export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
export OSS_ENDPOINT="oss-cn-hangzhou.aliyuncs.com"
export OSS_REGION="cn-hangzhou"
```

#### List Buckets

```cpp
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <iostream>

using namespace alibabacloud::oss2;

int main() {
    // Initialize client configuration
    auto conf = ClientConfiguration::loadDefault();
    conf.region = "cn-hangzhou";
    conf.credentialsProvider = std::make_shared<EnvironmentVariableCredentialsProvider>();

    // Create OSS client
    OSSClient client(conf);

    // List buckets
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
        std::cout << "Bucket: " << bucket.name
                  << ", Location: " << bucket.location
                  << ", StorageClass: " << bucket.storageClass << std::endl;
    }
    return 0;
}
```

#### List Objects

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
        std::cout << "Object: " << object.key
                  << ", Size: " << object.size
                  << ", LastModified: " << object.lastModified << std::endl;
    }
    return 0;
}
```

#### Put Object

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
    std::cout << "Upload successful! statusCode: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    return 0;
}
```

#### Get Object

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

    std::cout << "Download successful! Content: " << buffer.str() << std::endl;
    return 0;
}
```

## Complete Examples

More example projects can be found in the `samples` folder.

### Running Examples

Configure credentials from environment variables:

```bash
export OSS_ACCESS_KEY_ID="your_access_key_id"
export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
```

#### API & Paginator Samples

Build from the project root with `BUILD_SAMPLES=ON`:

```bash
cmake -B build -DBUILD_SAMPLES=ON
cmake --build build --config Release
```

Run a sample (all samples require `--region`; most require `--bucket` and `--key`):

```bash
./build/samples/sample_api_sync_PutObject --region cn-hangzhou --bucket my-bucket --key my-key
./build/samples/sample_api_async_PutObject --region cn-hangzhou --bucket my-bucket --key my-key
./build/samples/sample_paginator_ListObjectsV2Paginator --region cn-hangzhou --bucket my-bucket
```

You can also build a subset using `SAMPLE_FILTER`:

```bash
cmake -B build -DBUILD_SAMPLES=ON -DSAMPLE_FILTER=api/sync
cmake -B build -DBUILD_SAMPLES=ON -DSAMPLE_FILTER=paginator
```

#### Scenario Samples

Scenario samples under `samples/scenario/` are **independent projects** with their own `CMakeLists.txt`. They are NOT built by `BUILD_SAMPLES=ON`. Install the SDK first, then build them separately:

```bash
cd samples/scenario/progress
cmake -B build -DCMAKE_PREFIX_PATH=<sdk-install-prefix>
cmake --build build
```

Available scenarios: upload progress, curl/WinHTTP transport customization, custom retry strategies, RequestBody variants, request cancellation, endpoint configuration, async-on-sync-client, and credential providers. See [`samples/INDEX.md`](samples/INDEX.md) for the full list.

## Error Handling

The SDK uses `Outcome<Result, Error>` which provides an interface compatible with `std::expected`:

```cpp
auto outcome = client.putObject(request);

if (!outcome.has_value()) {
    auto& err = outcome.error();
    std::cerr << "Error Code: " << err.getCode() << std::endl;
    std::cerr << "Error Message: " << err.getMessage() << std::endl;
    std::cerr << "Request ID: " << err.getRequestId() << std::endl;
    std::cerr << "Operation: " << err.getOpName() << std::endl;
    std::cerr << "Method: " << err.getMethod() << std::endl;
}
```

When built with `-DUSE_STD_EXPECTED=ON` (C++23), `Outcome` becomes a type alias for `std::expected`, enabling monadic operations like `.and_then()`, `.transform()`, and `.or_else()`.

The legacy interface (`isSuccess()` / `getResult()` / `getError()`) is also available for compatibility with users migrating from [aliyun-oss-cpp-sdk](https://github.com/aliyun/aliyun-oss-cpp-sdk). Note that the legacy interface is not available in `std::expected` mode.

```cpp
auto outcome = client.putObject(request);

if (!outcome.isSuccess()) {
    auto& error = outcome.getError();
    std::cerr << "Error Code: " << error.getCode() << std::endl;
    std::cerr << "Error Message: " << error.getMessage() << std::endl;
}

auto& result = outcome.getResult();
```

## Thread Safety

The `OSSClient` instance is thread-safe and can be shared across multiple threads. However, request and result objects are not thread-safe and should not be shared between threads.

## License

> - Apache-2.0, see [license file](LICENSE)
