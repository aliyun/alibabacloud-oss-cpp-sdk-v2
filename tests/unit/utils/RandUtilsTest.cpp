#include <gtest/gtest.h>

#include "src/utils/Utils.h"

#include <set>
#include <thread>
#include <vector>
#include <mutex>

namespace alibabacloud {
namespace oss2 {
namespace utils {

TEST(RandUtilsTest, BasicNonZero) {
    // Multiple calls should not all return 0
    bool hasNonZero = false;
    for (int i = 0; i < 100; i++) {
        if (GetRandomValue() != 0) {
            hasNonZero = true;
            break;
        }
    }
    EXPECT_TRUE(hasNonZero);
}

TEST(RandUtilsTest, NotConstant) {
    // Multiple calls should produce different values
    std::set<uint32_t> values;
    for (int i = 0; i < 100; i++) {
        values.insert(GetRandomValue());
    }
    // At least 2 distinct values out of 100 calls
    EXPECT_GT(values.size(), 1u);
}

TEST(RandUtilsTest, ThreadSafety) {
    // Call from multiple threads concurrently to verify no crash or data race
    constexpr int numThreads = 8;
    constexpr int callsPerThread = 1000;

    std::vector<std::thread> threads;
    std::mutex mtx;
    std::set<uint32_t> allValues;

    for (int t = 0; t < numThreads; t++) {
        threads.emplace_back([&]() {
            std::set<uint32_t> localValues;
            for (int i = 0; i < callsPerThread; i++) {
                localValues.insert(GetRandomValue());
            }
            std::lock_guard<std::mutex> lock(mtx);
            allValues.insert(localValues.begin(), localValues.end());
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // With 8 threads x 1000 calls, we should have many distinct values
    EXPECT_GT(allValues.size(), 100u);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
