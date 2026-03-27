#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/BucketBasic.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(GetBucketStatTest, EmptyConstructor) {
    auto stat = BucketStat();
    EXPECT_FALSE(stat.archiveMultipartPartCount.has_value());

    stat.setArchiveMultipartPartCount(10LL);
    EXPECT_TRUE(stat.archiveMultipartPartCount.has_value());
    EXPECT_EQ(10, stat.archiveMultipartPartCount.value());
}

TEST(GetBucketStatTest, ResultConstructor) {
    auto result = GetBucketStatResult();
    EXPECT_FALSE(result.hasBucketStat());
    EXPECT_FALSE(result.getBucketStat().archiveMultipartPartCount.has_value());

    auto stat1 = BucketStat();
    stat1.setArchiveMultipartPartCount(10LL);
    EXPECT_TRUE(stat1.archiveMultipartPartCount.has_value());

    result.setBucketStat(std::move(stat1));
    EXPECT_TRUE(result.hasBucketStat());
    EXPECT_TRUE(result.getBucketStat().archiveMultipartPartCount.has_value());
    EXPECT_EQ(10, result.getBucketStat().archiveMultipartPartCount.value());
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud
