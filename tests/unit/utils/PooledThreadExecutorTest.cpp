#include <gtest/gtest.h>

#include "alibabacloud/oss2/utils/PooledThreadExecutor.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <set>

namespace alibabacloud::oss2 {

TEST(PooledThreadExecutorTest, BasicExecution) {
    std::atomic<int> counter(0);
    {
        PooledThreadExecutor exec(4);
        for (int i = 0; i < 100; ++i) {
            exec.execute([&] { counter++; });
        }
    }
    EXPECT_EQ(100, counter.load());
}

TEST(PooledThreadExecutorTest, TasksRunOnPoolThreads) {
    constexpr size_t poolSize = 4;
    std::mutex mtx;
    std::set<std::thread::id> threadIds;
    {
        PooledThreadExecutor exec(poolSize);
        for (int i = 0; i < 20; ++i) {
            exec.execute([&] {
                std::lock_guard<std::mutex> lock(mtx);
                threadIds.insert(std::this_thread::get_id());
            });
        }
    }
    EXPECT_GT(threadIds.size(), 0u);
    EXPECT_LE(threadIds.size(), poolSize);
}

TEST(PooledThreadExecutorTest, ConcurrentTasks) {
    constexpr int taskCount = 1000;
    std::atomic<int> counter(0);
    {
        PooledThreadExecutor exec(8);
        for (int i = 0; i < taskCount; ++i) {
            exec.execute([&] { counter++; });
        }
    }
    EXPECT_EQ(taskCount, counter.load());
}

TEST(PooledThreadExecutorTest, TasksAfterShutdownAreDropped) {
    std::atomic<int> counter(0);
    auto exec = std::make_unique<PooledThreadExecutor>(2);
    exec->execute([&] { counter++; });
    exec.reset();
    EXPECT_EQ(1, counter.load());
}

TEST(PooledThreadExecutorTest, ConcurrentSubmitAndShutdown) {
    constexpr size_t repeats = 20;
    for (size_t r = 0; r < repeats; ++r) {
        auto exec = std::make_shared<PooledThreadExecutor>(4);
        std::atomic<size_t> counter(0);

        std::mutex mtx;
        std::condition_variable cv;
        bool go = false;

        constexpr size_t submitters = 4;
        std::vector<std::future<void>> futures;
        std::atomic<size_t> readyCount(0);

        for (size_t i = 0; i < submitters; ++i) {
            auto localExec = exec;
            futures.push_back(std::async(std::launch::async, [localExec, &counter, &readyCount, &mtx, &cv, &go] {
                readyCount++;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [&] { return go; });
                }
                localExec->execute([&counter] { counter++; });
            }));
        }

        futures.push_back(std::async(std::launch::async, [&exec, &readyCount, &mtx, &cv, &go] {
            readyCount++;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] { return go; });
            }
            exec.reset();
        }));

        while (readyCount.load() < submitters + 1) {
            std::this_thread::yield();
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            go = true;
        }
        cv.notify_all();

        for (auto& f : futures) {
            f.get();
        }
    }
}

} // namespace alibabacloud::oss2
