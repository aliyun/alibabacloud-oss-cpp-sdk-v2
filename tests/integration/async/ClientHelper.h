#pragma once

#include "alibabacloud/oss2/OSSAsyncClient.h"
#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace async {

class ClientHelper {
  public:
    static std::shared_ptr<OSSAsyncClient> GetDefaultClient();
    static std::shared_ptr<OSSAsyncClient> GetInvalidClient();
    static void CleanBucket(const std::string& bucketName);
    static void CleanVersioningBucket(const std::string& bucketName);
    static void CleanBucketsByPrefix(const std::string& prefix);
};

} // namespace async
} // namespace oss2
} // namespace alibabacloud
