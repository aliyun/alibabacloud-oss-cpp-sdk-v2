
#include "alibabacloud/oss2/utils/Cancellation.h"

namespace alibabacloud {
namespace oss2 {

bool CancellationToken::waitFor(std::chrono::milliseconds timeout) const {
    auto src = source_.lock();
    if (!src) return true;

    std::unique_lock<std::mutex> lk(src->mu_);
    if (isCanceled()) return true;

    auto now = std::chrono::steady_clock::now();
    auto timeToDeadline = std::chrono::duration_cast<std::chrono::milliseconds>(deadline_->load() - now);
    auto waitDuration = (std::min)(timeout, timeToDeadline);

    if (waitDuration.count() > 0) {
        src->cv_.wait_for(lk, waitDuration, [this] { return isCanceled(); });
    }
    return isCanceled();
}

} // namespace oss2
} // namespace alibabacloud
