#include "DefaultScheduledExecutor.h"

namespace alibabacloud {
namespace oss2 {

DefaultScheduledExecutor::DefaultScheduledExecutor() : impl_(std::make_shared<Impl>()) {
    impl_->thread = std::thread([impl = impl_]() { run(impl); });
}

DefaultScheduledExecutor::~DefaultScheduledExecutor() {
    {
        std::lock_guard<std::mutex> lk(impl_->mutex);
        if (impl_->stop) {
            return;
        }
        impl_->stop = true;
    }
    impl_->cv.notify_one();
    if (impl_->thread.get_id() == std::this_thread::get_id()) {
        impl_->thread.detach();
    } else {
        impl_->thread.join();
    }
}

void DefaultScheduledExecutor::schedule(std::chrono::milliseconds delay, std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lk(impl_->mutex);
        if (impl_->stop) {
            return;
        }
        impl_->queue.push({std::chrono::steady_clock::now() + delay, std::move(task)});
    }
    impl_->cv.notify_one();
}

void DefaultScheduledExecutor::run(std::shared_ptr<Impl> impl) {
    std::unique_lock<std::mutex> lk(impl->mutex);
    for (;;) {
        if (impl->stop) {
            return;
        }
        if (impl->queue.empty()) {
            impl->cv.wait(lk, [&]() { return impl->stop || !impl->queue.empty(); });
            continue;
        }
        auto deadline = impl->queue.top().deadline;
        if (impl->cv.wait_until(lk, deadline, [&]() { return impl->stop; })) {
            return;
        }
        while (!impl->queue.empty() && impl->queue.top().deadline <= std::chrono::steady_clock::now()) {
            auto task = std::move(const_cast<std::function<void()>&>(impl->queue.top().task));
            impl->queue.pop();
            lk.unlock();
            task();
            task = nullptr;
            lk.lock();
        }
    }
}

} // namespace oss2
} // namespace alibabacloud
