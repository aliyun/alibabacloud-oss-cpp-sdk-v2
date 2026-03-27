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

> - C++17 or later
> - CMake 3.10 or later
> - Supported platforms: Linux, macOS, Windows

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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "ListBuckets fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "ListObjectsV2 fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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
    if (!outcome.isSuccess()) {
        auto& e = outcome.getError();
        std::cerr << "PutObject fail, code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.getResult();
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

    std::cout << "Download successful! Content: " << buffer.str() << std::endl;
    return 0;
}
```

## Complete Examples

More example projects can be found in the `samples` folder.

### Running Examples

> - Go to the sample code folder `samples`.
> - Configure credentials from environment variables:
>   ```bash
>   export OSS_ACCESS_KEY_ID="your_access_key_id"
>   export OSS_ACCESS_KEY_SECRET="your_access_key_secret"
>   export OSS_REGION="cn-hangzhou"
>   ```
> - Build the examples from the project root directory:
>   ```bash
>   mkdir build && cd build
>   cmake .. -DBUILD_SAMPLES=ON
>   cmake --build .
>   ```
> - Run an example:
>   ```bash
>   ./samples/sample_ListBuckets --region cn-hangzhou
>   ```

## Error Handling

The SDK provides detailed error information through the `Outcome` pattern:

```cpp
auto outcome = client.putObject(request);

if (!outcome.isSuccess()) {
    auto& error = outcome.getError();
    std::cerr << "Error Code: " << error.getCode() << std::endl;
    std::cerr << "Error Message: " << error.getMessage() << std::endl;
    std::cerr << "Request ID: " << error.getRequestId() << std::endl;
    std::cerr << "Operation: " << error.getOpName() << std::endl;
    std::cerr << "Method: " << error.getMethod() << std::endl;
}
```

## Thread Safety

The `OSSClient` instance is thread-safe and can be shared across multiple threads. However, request and result objects are not thread-safe and should not be shared between threads.

## License

> - Apache-2.0, see [license file](LICENSE)
