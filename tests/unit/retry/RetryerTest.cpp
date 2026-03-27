#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/Retryer.h"


#include <iostream>

using namespace alibabacloud::oss2;

TEST(RetryerTest, NopRetryer) {
    auto retryer = NopRetryer();
    auto error = std::error_code();
    EXPECT_EQ(1L, retryer.getMaxAttempts());
    EXPECT_EQ(std::chrono::microseconds(0), retryer.calcDelayTime(error, 0L));
    EXPECT_EQ(false, retryer.isErrorRetryable(error));
}

TEST(RetryerTest, EqualJitterBackoff) {
    auto delayBase = std::chrono::milliseconds(200);
    auto maxDelay = std::chrono::milliseconds(20000);
    auto backoff = EqualJitterBackoff(delayBase, maxDelay);
    for (int i = 0; i < 128; i++) {
        auto delay = backoff.backoffDelay(i, std::error_code()).count();
        if (i == 0) {
            EXPECT_GE(delay, 0LL);
        } else {
            EXPECT_GT(delay, 0LL);
        }
        EXPECT_LT(delay, maxDelay.count());
    }
}

TEST(RetryerTest, FullJitterBackoff) {
    auto delayBase = std::chrono::milliseconds(200);
    auto maxDelay = std::chrono::milliseconds(20000);
    auto backoff = FullJitterBackoff(delayBase, maxDelay);
    for (int i = 0; i < 128; i++) {
        auto delay = backoff.backoffDelay(i, std::error_code()).count();
        if (i == 0) {
            EXPECT_GE(delay, 0LL);
        } else {
            EXPECT_GT(delay, 0LL);
        }
        EXPECT_LT(delay, maxDelay.count());
    }
}

TEST(RetryerTest, FixedDelayBackoff) {
    auto delay = std::chrono::milliseconds(200);
    auto backoff = FixedDelayBackoff(delay);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(delay, backoff.backoffDelay(i, std::error_code()));
    }
}