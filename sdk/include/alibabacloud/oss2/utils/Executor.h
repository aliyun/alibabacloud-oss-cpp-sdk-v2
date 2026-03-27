
#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include "alibabacloud/oss2/utils/Runnable.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>


namespace alibabacloud {
namespace oss2 {
class ALIBABACLOUD_OSS_API Executor {
  public:
    virtual ~Executor() = default;
    virtual void execute(Runnable* task) = 0;
};
} // namespace oss2
} // namespace alibabacloud