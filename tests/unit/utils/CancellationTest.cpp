#include <gtest/gtest.h>

#include "alibabacloud/oss2/utils/Cancellation.h"

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

} // namespace alibabacloud::oss2