#include <gtest/gtest.h>

#include "alibabacloud/oss2/utils/Cancellation.h"

#include <thread>

namespace alibabacloud::oss2 {

TEST(CancellationTest, CancellationTokenSource) {
    auto cts = CancellationTokenSource::create();
    EXPECT_NE(nullptr, cts);
    auto deadline = cts->getDeadline();
    EXPECT_EQ(std::chrono::steady_clock::time_point::max(), deadline);

    // dealy duration: 100s later
    cts->cancelAfter(std::chrono::seconds(100));
    auto now = std::chrono::steady_clock::now();
    deadline = cts->getDeadline();
    EXPECT_NE(std::chrono::steady_clock::time_point::max(), deadline);
    EXPECT_LT(deadline, now + std::chrono::seconds(150));
    EXPECT_GT(deadline, now + std::chrono::seconds(98));

    // time_point: user's time view 50s later
    auto usernow = std::chrono::system_clock::now();
    now = std::chrono::steady_clock::now();
    cts->cancelAfter(usernow + std::chrono::seconds(50));
    deadline = cts->getDeadline();
    EXPECT_LT(deadline, now + std::chrono::seconds(60));
    EXPECT_GT(deadline, now + std::chrono::seconds(48));

    // cancel immediately
    cts->cancel();
    now = std::chrono::steady_clock::now();
    deadline = cts->getDeadline();
    EXPECT_LE(deadline, now);
}

TEST(CancellationTest, CancellationToken) {
    auto ct = CancellationToken();
    EXPECT_FALSE(ct.canBeCanceled());
    EXPECT_FALSE(ct.isCanceled());
}


TEST(CancellationTest, CancellationTokenFromSource) {
    auto cts = CancellationTokenSource::create();
    auto ct = cts->getToken();

    EXPECT_TRUE(ct.canBeCanceled());
    EXPECT_FALSE(ct.isCanceled());

    cts->cancelAfter(std::chrono::seconds(100));
    auto now = std::chrono::steady_clock::now();
    auto deadline = cts->getDeadline();
    EXPECT_NE(std::chrono::steady_clock::time_point::max(), deadline);
    EXPECT_LT(deadline, now + std::chrono::seconds(150));
    EXPECT_GT(deadline, now + std::chrono::seconds(98));
    EXPECT_FALSE(ct.isCanceled());

    cts->cancel();
    now = std::chrono::steady_clock::now();
    deadline = cts->getDeadline();
    EXPECT_LE(deadline, now);
    EXPECT_TRUE(ct.isCanceled());
}

TEST(CancellationTest, WaitFor_Timeout) {
    auto cts = CancellationTokenSource::create();
    auto ct = cts->getToken();

    auto start = std::chrono::steady_clock::now();
    bool canceled = ct.waitFor(std::chrono::milliseconds(50));
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(canceled);
    EXPECT_GE(elapsed, std::chrono::milliseconds(45));
}

TEST(CancellationTest, WaitFor_CancelImmediately) {
    auto cts = CancellationTokenSource::create();
    auto ct = cts->getToken();

    std::thread t([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        cts->cancel();
    });

    auto start = std::chrono::steady_clock::now();
    bool canceled = ct.waitFor(std::chrono::milliseconds(2000));
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(canceled);
    EXPECT_LT(elapsed, std::chrono::milliseconds(200));
    t.join();
}

TEST(CancellationTest, WaitFor_CancelAfterDeadline) {
    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::milliseconds(50));
    auto ct = cts->getToken();

    auto start = std::chrono::steady_clock::now();
    bool canceled = ct.waitFor(std::chrono::milliseconds(2000));
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(canceled);
    EXPECT_GE(elapsed, std::chrono::milliseconds(45));
    EXPECT_LT(elapsed, std::chrono::milliseconds(200));
}

TEST(CancellationTest, WaitFor_AlreadyCanceled) {
    auto cts = CancellationTokenSource::create();
    cts->cancel();
    auto ct = cts->getToken();

    auto start = std::chrono::steady_clock::now();
    bool canceled = ct.waitFor(std::chrono::milliseconds(1000));
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(canceled);
    EXPECT_LT(elapsed, std::chrono::milliseconds(50));
}

TEST(CancellationTest, WaitFor_NoSource) {
    CancellationToken ct;
    bool canceled = ct.waitFor(std::chrono::milliseconds(50));
    EXPECT_TRUE(canceled);
}

TEST(CancellationTest, WaitFor_SourceDestroyed) {
    auto cts = CancellationTokenSource::create();
    auto ct = cts->getToken();
    cts.reset();

    bool canceled = ct.waitFor(std::chrono::milliseconds(50));
    EXPECT_TRUE(canceled);
}

TEST(CancellationTest, WaitFor_CancelAfterDuringWait_NotShortened) {
    auto cts = CancellationTokenSource::create();
    auto ct = cts->getToken();

    std::chrono::steady_clock::duration elapsed{};
    bool canceled = false;

    std::thread t([&ct, &elapsed, &canceled]() {
        auto start = std::chrono::steady_clock::now();
        canceled = ct.waitFor(std::chrono::milliseconds(200));
        elapsed = std::chrono::steady_clock::now() - start;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    cts->cancelAfter(std::chrono::milliseconds(50));

    t.join();

    // cancelAfter() does not wake waitFor(), but deadline passes during wait
    EXPECT_TRUE(canceled);
#ifdef __MINGW32__
    // On MinGW, The predicate re-evaluates after the cancelAfter deadline passes,
    // causing early return.
    EXPECT_GE(elapsed, std::chrono::milliseconds(80));
#else
    EXPECT_GE(elapsed, std::chrono::milliseconds(180));
#endif
}

TEST(CancellationTest, WaitFor_CancelAfterBeforeWait_Shortened) {
    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::milliseconds(50));
    auto ct = cts->getToken();

    auto start = std::chrono::steady_clock::now();
    bool canceled = ct.waitFor(std::chrono::milliseconds(2000));
    auto elapsed = std::chrono::steady_clock::now() - start;

    // cancelAfter() set before waitFor(), so waitFor computes min(2000, ~50) and sleeps ~50ms
    EXPECT_TRUE(canceled);
    EXPECT_GE(elapsed, std::chrono::milliseconds(45));
    EXPECT_LT(elapsed, std::chrono::milliseconds(200));
}

} // namespace alibabacloud::oss2