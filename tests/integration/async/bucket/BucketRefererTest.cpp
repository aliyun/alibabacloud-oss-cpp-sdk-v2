#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

TEST(AsyncBucketRefererTest, PutBucketReferer_normal) {
    auto client = ClientHelper::GetDefaultClient();
    auto bucket = Config::GenBucketName();
    auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucket));
    EXPECT_TRUE(future.get().has_value());

    auto rrequest = models::PutBucketRefererRequest();
    rrequest.setBucket(bucket);
    auto refererConfiguration = models::RefererConfiguration();
    refererConfiguration.setAllowEmptyReferer(true);
    refererConfiguration.setRefererList({{"http://www.aliyun.com", "https://www.aliyun.com"}});
    rrequest.setRefererConfiguration(std::move(refererConfiguration));
    auto rfuture = client->asyncCall(rrequest);
    EXPECT_TRUE(rfuture.get().has_value());

    auto grequest = models::GetBucketRefererRequest();
    grequest.setBucket(bucket);
    auto gfuture = client->asyncCall(grequest);
    auto goutcome = gfuture.get();
    EXPECT_TRUE(goutcome.has_value());
    EXPECT_TRUE(goutcome.value().hasRefererConfiguration());
    auto& config = goutcome.value().getRefererConfiguration();
    EXPECT_TRUE(config.allowEmptyReferer.has_value());
    EXPECT_TRUE(config.refererList.has_value());

    ClientHelper::CleanBucketsByPrefix(bucket);
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
