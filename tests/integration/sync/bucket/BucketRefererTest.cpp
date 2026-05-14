#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

TEST(BucketRefererTest, PutBucketReferer_normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto bucket = Config::GenBucketName();
    auto request = models::PutBucketRequest();
    request.setBucket(bucket);
    auto outcome = client->putBucket(request);
    EXPECT_TRUE(outcome.has_value());

    auto rrequest = models::PutBucketRefererRequest();
    rrequest.setBucket(bucket);
    auto refererConfiguration = models::RefererConfiguration();
    refererConfiguration.setAllowEmptyReferer(true);
    refererConfiguration.setRefererList({{"http://www.aliyun.com", "https://www.aliyun.com"}});
    rrequest.setRefererConfiguration(std::move(refererConfiguration));
    auto routcome = client->putBucketReferer(rrequest);
    EXPECT_TRUE(routcome.has_value());

    auto grequest = models::GetBucketRefererRequest();
    grequest.setBucket(bucket);
    auto goutcome = client->getBucketReferer(grequest);
    EXPECT_TRUE(goutcome.has_value());
    EXPECT_TRUE(goutcome.value().hasRefererConfiguration());
    auto& config = goutcome.value().getRefererConfiguration();
    EXPECT_TRUE(config.allowEmptyReferer.has_value());
    EXPECT_TRUE(config.allowTruncateQueryString.has_value());
    EXPECT_TRUE(config.refererList.has_value());
    for (auto& it : config.refererList.value().referers) {
        std::cout << it << std::endl;
    }
    EXPECT_FALSE(config.refererBlacklist.has_value());
}

TEST(BucketRefererTest, BucketReferer_fail) {}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
