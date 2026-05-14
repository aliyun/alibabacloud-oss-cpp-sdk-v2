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
    auto future = client->callAsync<PutBucketOutcome>(
        &OSSAsyncClient::putBucketAsync, models::PutBucketRequest().setBucket(bucket));
    EXPECT_TRUE(future.get().isSuccess());

    auto rrequest = models::PutBucketRefererRequest();
    rrequest.setBucket(bucket);
    auto refererConfiguration = models::RefererConfiguration();
    refererConfiguration.setAllowEmptyReferer(true);
    refererConfiguration.setRefererList({{"http://www.aliyun.com", "https://www.aliyun.com"}});
    rrequest.setRefererConfiguration(std::move(refererConfiguration));
    auto rfuture = client->callAsync<PutBucketRefererOutcome>(&OSSAsyncClient::putBucketRefererAsync, rrequest);
    EXPECT_TRUE(rfuture.get().isSuccess());

    auto grequest = models::GetBucketRefererRequest();
    grequest.setBucket(bucket);
    auto gfuture = client->callAsync<GetBucketRefererOutcome>(&OSSAsyncClient::getBucketRefererAsync, grequest);
    auto goutcome = gfuture.get();
    EXPECT_TRUE(goutcome.isSuccess());
    EXPECT_TRUE(goutcome.getResult().hasRefererConfiguration());
    auto& config = goutcome.getResult().getRefererConfiguration();
    EXPECT_TRUE(config.allowEmptyReferer.has_value());
    EXPECT_TRUE(config.refererList.has_value());

    ClientHelper::CleanBucketsByPrefix(bucket);
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
