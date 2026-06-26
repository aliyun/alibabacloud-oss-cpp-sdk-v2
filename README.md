# Alibaba Cloud OSS SDK for C++ V2

[![GitHub version](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2.svg)](https://badge.fury.io/gh/aliyun%2Falibabacloud-oss-cpp-sdk-v2)

alibabacloud-oss-cpp-sdk-v2 is the developer preview for the v2 of the OSS SDK for the C++ programming language

## [中文文档](README_CN.md)

## About

> - This C++ SDK is based on the official APIs of [Alibaba Cloud OSS](http://www.aliyun.com/product/oss/).
> - Alibaba Cloud Object Storage Service (OSS) is a cloud storage service provided by Alibaba Cloud, featuring massive capacity, security, a low cost, and high reliability.
> - The OSS can store any type of files and therefore applies to various websites, development enterprises and developers.
> - With this SDK, you can upload, download and manage data on any app anytime and anywhere conveniently.

## Requirements

> - C++17 or later (C++23 required for `std::expected` mode)
> - CMake 3.15 or later
> - Supported platforms: Linux, macOS, Windows, Android

## Features

- **Typed API Coverage** -- object, multipart upload, bucket management, etc.
- **Generic API** -- `invokeOperation()` for calling any OSS API without typed request/response models
- **Sync & Async** -- `OSSClient` for synchronous calls; `OSSAsyncClient` for fully asynchronous operations; `OSSClient::asyncCall()` for running sync operations on an executor
- **Automatic Retry** -- configurable retry with exponential backoff for transient failures
- **Paginator** -- `makePaginator()` for automatic page iteration over list operations
- **Presigned URLs** -- generate presigned URLs for PutObject, GetObject, HeadObject, UploadPart
- **Request Cancellation** -- `CancellationToken` for canceling in-flight requests; `disableRequest()` for client-level bulk cancellation
- **Progress Callback** -- track upload/download progress
- **Flexible Request Body** -- construct from String, File, Stream, or Memory buffer
- **Multiple Credential Providers** -- environment variables, static credentials, STS, ECS RAM role, custom providers
- **Client-Side Encryption** -- `OSSEncryptionClient` for transparent encrypt-on-upload and decrypt-on-download
- **Customizable Transport** -- plug in custom Curl or WinHTTP configurations
- **`std::expected` Support** -- optional C++23 mode with monadic error handling

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
| `USE_CURL_TRANSPORT` | `ON` | Enable libcurl HTTP transport |
| `USE_WINHTTP_TRANSPORT` | `OFF` | Enable WinHTTP transport (Windows only) |
| `USE_SYSTEM_CURL` | `OFF` | Use system-installed libcurl |
| `USE_SYSTEM_OPENSSL` | `OFF` | Use system-installed OpenSSL |
| `USE_SYSTEM_MBEDTLS` | `OFF` | Use system-installed mbedTLS |
| `USE_SYSTEM_TINYXML2` | `OFF` | Use system-installed tinyxml2 |
| `USE_STD_EXPECTED` | `OFF` | Use `std::expected` instead of custom `Outcome` (requires C++23) |
| `ENABLE_ENCRYPTION` | `OFF` | Enable client-side encryption (requires OpenSSL, mbedTLS, or Windows) |
| `ENABLE_RTTI` | `OFF` | Enable/disable building code with RTTI information |
| `ENABLE_COVERAGE` | `OFF` | Generate coverage reports |
| `ENABLE_CPPCHECK` | `OFF` | Enable Cppcheck static analysis |
| `ENABLE_SANITIZER` | `OFF` | Enable sanitizers |

To enable `std::expected` mode (C++23):

```bash
cmake -B build -DCMAKE_CXX_STANDARD=23 -DUSE_STD_EXPECTED=ON
cmake --build build
```

### Install via vcpkg

```bash
vcpkg install alibabacloud-oss-cpp-sdk-v2[curl]

# Or on Windows with WinHTTP
vcpkg install alibabacloud-oss-cpp-sdk-v2[winhttp]
```

You can also install from the source tree using an overlay port:

```bash
git clone https://github.com/aliyun/alibabacloud-oss-cpp-sdk-v2.git
vcpkg install alibabacloud-oss-cpp-sdk-v2[curl] --overlay-ports=alibabacloud-oss-cpp-sdk-v2/vcpkg --head
```

Available features: `curl`, `winhttp`, `openssl`, `mbedtls`, `rtti`, `encryption`, `tinyxml2`.

**Note:**
- An HTTP transport (`curl` or `winhttp`) must be explicitly specified. Both can be enabled simultaneously.
- When both `openssl` and `mbedtls` are enabled, `openssl` takes precedence.

### Using CMake in your project

Add the following to your `CMakeLists.txt`:

```cmake
find_package(alibabacloud_oss_v2 REQUIRED)

target_link_libraries(your_target PRIVATE alibabacloud_oss_v2::oss)
```

## Quick Start

Set your credentials via environment variables before running:

```bash
export OSS_ACCESS_KEY_ID="your_access_key_id"
export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
```

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

    // Upload
    auto putOutcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key")
            .setBody(oss::RequestBody::fromString("Hello, OSS!")));
    if (!putOutcome.has_value()) {
        auto& err = putOutcome.error();
        std::cerr << "Error Code: " << err.getCode() << std::endl;
        std::cerr << "Error Message: " << err.getMessage() << std::endl;
        std::cerr << "EC: " << err.getEC() << std::endl;
        std::cerr << "Request ID: " << err.getRequestId() << std::endl;
        std::cerr << "Request Target: " << err.getRequestTarget() << std::endl;
        return 1;
    }
    std::cout << "Upload successful, ETag: " << putOutcome.value().getETag() << std::endl;

    // Download
    auto getOutcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket("your-bucket-name")
            .setKey("your-object-key"));
    if (!getOutcome.has_value()) {
        auto& err = getOutcome.error();
        std::cerr << "Error Code: " << err.getCode() << std::endl;
        std::cerr << "Error Message: " << err.getMessage() << std::endl;
        std::cerr << "EC: " << err.getEC() << std::endl;
        std::cerr << "Request ID: " << err.getRequestId() << std::endl;
        std::cerr << "Request Target: " << err.getRequestTarget() << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << getOutcome.value().getBody()->rdbuf();
    std::cout << "Download successful, content: " << buffer.str() << std::endl;

    return 0;
}
```

## Error Handling

The SDK uses `Outcome<Result, Error>` which provides an interface compatible with `std::expected`:

```cpp
auto outcome = client.putObject(request);

if (!outcome.has_value()) {
    auto& err = outcome.error();
    std::cerr << "Error Code: " << err.getCode() << std::endl;
    std::cerr << "Error Message: " << err.getMessage() << std::endl;
    std::cerr << "EC: " << err.getEC() << std::endl;
    std::cerr << "Request ID: " << err.getRequestId() << std::endl;
    std::cerr << "Request Target: " << err.getRequestTarget() << std::endl;
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

Both `OSSClient` and `OSSAsyncClient` instances are thread-safe and can be shared across multiple threads. However, request and result objects are not thread-safe and should not be shared between threads.

## Complete Examples

More examples can be found in the [`samples`](samples/INDEX.md) directory, including sync/async API usage, paginators, and scenario samples (progress callback, transport customization, retry strategies, request cancellation, credential providers, etc.).

## Migration Guide

If you are upgrading from V1 ([aliyun-oss-cpp-sdk](https://github.com/aliyun/aliyun-oss-cpp-sdk)), see the [Migration Guide](MIGRATION.md) for a detailed comparison of API changes, configuration, error handling, and more.

## License

> - Apache-2.0, see [license file](LICENSE)
