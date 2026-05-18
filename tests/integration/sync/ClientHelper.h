#pragma once

#include "alibabacloud/oss2/OSSClient.h"
#include <memory>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ClientHelper {
  public:
    static std::shared_ptr<OSSClient> GetDefaultClient();
    static std::shared_ptr<OSSClient> GetInvalidClient();
    static void CleanBucket(const std::string& bucketName);
    static void CleanVersioningBucket(const std::string& bucketName);
    static void CleanBucketsByPrefix(const std::string& prefix);
};

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
