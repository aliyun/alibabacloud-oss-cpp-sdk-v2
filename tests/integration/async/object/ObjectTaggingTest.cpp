#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObjectTaggingTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->callAsync<PutBucketOutcome>(&OSSAsyncClient::putBucketAsync,
                                                          models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().isSuccess());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncObjectTaggingTest::bucketName_ = "";

TEST_F(AsyncObjectTaggingTest, ObjectTagging_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-tagging-object";

    auto putFuture = client->callAsync<PutObjectOutcome>(
        &OSSAsyncClient::putObjectAsync,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(RequestBody::FromString("tagging content")));
    EXPECT_TRUE(putFuture.get().isSuccess());

    std::vector<models::Tag> tags;
    models::Tag tag1;
    tag1.setKey("env");
    tag1.setValue("test");
    tags.push_back(tag1);
    models::Tag tag2;
    tag2.setKey("project");
    tag2.setValue("oss-sdk");
    tags.push_back(tag2);

    models::TagSet tagSet;
    tagSet.setTags(tags);
    models::Tagging tagging;
    tagging.setTagSet(tagSet);

    auto putTagFuture = client->callAsync<PutObjectTaggingOutcome>(
        &OSSAsyncClient::putObjectTaggingAsync,
        models::PutObjectTaggingRequest().setBucket(bucketName_).setKey(key).setTagging(tagging));
    EXPECT_TRUE(putTagFuture.get().isSuccess());

    auto getTagFuture = client->callAsync<GetObjectTaggingOutcome>(
        &OSSAsyncClient::getObjectTaggingAsync,
        models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getTagFuture.get();
    EXPECT_TRUE(getOutcome.isSuccess());
    auto& result = getOutcome.getResult();
    EXPECT_TRUE(result.hasTagging());
    auto& resultTagging = result.getTagging();
    EXPECT_TRUE(resultTagging.tagSet.has_value());
    EXPECT_EQ(2, resultTagging.tagSet.value().tags.size());
    EXPECT_EQ("env", resultTagging.tagSet.value().tags.at(0).key);
    EXPECT_EQ("test", resultTagging.tagSet.value().tags.at(0).value);

    auto delTagFuture = client->callAsync<DeleteObjectTaggingOutcome>(
        &OSSAsyncClient::deleteObjectTaggingAsync,
        models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(delTagFuture.get().isSuccess());

    Config::WaitForCacheExpire(2);
    auto getTag2Future = client->callAsync<GetObjectTaggingOutcome>(
        &OSSAsyncClient::getObjectTaggingAsync,
        models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome2 = getTag2Future.get();
    EXPECT_TRUE(getOutcome2.isSuccess());
    if (getOutcome2.getResult().hasTagging() && getOutcome2.getResult().getTagging().tagSet.has_value()) {
        EXPECT_EQ(0, getOutcome2.getResult().getTagging().tagSet.value().tags.size());
    }
}

TEST_F(AsyncObjectTaggingTest, PutObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    models::Tagging tagging;
    auto future = client->callAsync<PutObjectTaggingOutcome>(
        &OSSAsyncClient::putObjectTaggingAsync,
        models::PutObjectTaggingRequest().setBucket(bucketName_).setKey("test-key").setTagging(tagging));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectTaggingTest, GetObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<GetObjectTaggingOutcome>(
        &OSSAsyncClient::getObjectTaggingAsync,
        models::GetObjectTaggingRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectTaggingTest, DeleteObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<DeleteObjectTaggingOutcome>(
        &OSSAsyncClient::deleteObjectTaggingAsync,
        models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
