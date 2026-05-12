#include <gtest/gtest.h>

#include "alibabacloud/oss2/utils/DefaultExecutor.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>

namespace alibabacloud::oss2 {

TEST(DefaultExecutorTest, ThreadsJoinOnDestruction) {
    std::atomic<int> counter(1);
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    {
        DefaultExecutor exec;
        exec.execute([&] {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return ready; });
            counter++;
        });
        exec.execute([&] {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return ready; });
            counter++;
        });
        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
        }
        cv.notify_all();
    }
    counter = counter.load() * 10;
    EXPECT_EQ(30, counter.load());
}

TEST(DefaultExecutorTest, ThreadsDetachIfNotShuttingDown) {
    std::atomic<int> counter(1);
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    DefaultExecutor exec;
    exec.execute([&] {
        counter++;
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
        cv.notify_one();
    });
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return done; });
    }
    counter = counter.load() * 10;
    EXPECT_EQ(20, counter.load());
}

TEST(DefaultExecutorTest, MultipleTasks) {
    constexpr int taskCount = 100;
    std::atomic<int> counter(0);
    {
        DefaultExecutor exec;
        for (int i = 0; i < taskCount; ++i) {
            exec.execute([&] { counter++; });
        }
    }
    EXPECT_EQ(taskCount, counter.load());
}

TEST(DefaultExecutorTest, ConcurrentSubmitAndShutdown) {
    constexpr size_t repeats = 20;
    for (size_t r = 0; r < repeats; ++r) {
        auto exec = std::make_shared<DefaultExecutor>();
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
