
#include "alibabacloud/oss2/utils/Cancellation.h"

namespace alibabacloud {
namespace oss2 {

bool CancellationToken::waitFor(std::chrono::milliseconds timeout) const {
    auto src = source_.lock();
    if (!src) return true;

    std::unique_lock<std::mutex> lk(src->mu_);
    if (isCanceled()) return true;

    auto now = std::chrono::steady_clock::now();
    auto wakeTime = (std::min)(now + timeout, deadline_->load());

    if (wakeTime > now) {
        src->cv_.wait_until(lk, wakeTime, [this] { return isCanceled(); });
    }
    return isCanceled();
}

} // namespace oss2
} // namespace alibabacloud
