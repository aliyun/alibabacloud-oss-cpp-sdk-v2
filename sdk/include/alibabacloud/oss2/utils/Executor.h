
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <functional>


namespace alibabacloud {
namespace oss2 {

/// Abstract base class for async task executors.
/// Subclass this to provide custom threading strategies for OSSClient::callAsync.
class ALIBABACLOUD_OSS_API Executor {
  public:
    virtual ~Executor() = default;
    virtual void execute(std::function<void()> task) = 0;
};
} // namespace oss2
} // namespace alibabacloud
