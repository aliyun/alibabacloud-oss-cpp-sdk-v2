#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectTaggingTest : public ::testing::Test {
  protected:
    ObjectTaggingTest() {}

    ~ObjectTaggingTest() override {}

    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string ObjectTaggingTest::bucketName_ = "";

// Object Tagging Tests
TEST_F(ObjectTaggingTest, ObjectTagging_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-tagging-object";
    std::string content = "Tagging test content";

    // Put object first
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putOutcome.has_value());

    // Create tags
    std::vector<models::Tag> tags;
    models::Tag tag1;
    tag1.setKey("env");
    tag1.setValue("test");
    tags.push_back(tag1);

    models::Tag tag2;
    std::string key1 = "project";
    tag2.setKey(key1);
    tag2.setValue("oss-sdk");
    tags.push_back(tag2);

    models::TagSet tagSet;
    tagSet.setTags(tags);

    models::Tagging tagging;
    tagging.setTagSet(tagSet);

    // Put object tagging
    auto putTaggingOutcome = client->putObjectTagging(
            models::PutObjectTaggingRequest().setBucket(bucketName_).setKey(key).setTagging(tagging));
    EXPECT_TRUE(putTaggingOutcome.has_value());

    // Get object tagging
    auto getTaggingOutcome =
            client->getObjectTagging(models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(getTaggingOutcome.has_value());
    auto& result = getTaggingOutcome.value();
    EXPECT_TRUE(result.hasTagging());
    auto& resultTagging = result.getTagging();
    EXPECT_TRUE(resultTagging.tagSet.has_value());
    EXPECT_EQ(2, resultTagging.tagSet.value().tags.size());
    EXPECT_EQ("env", resultTagging.tagSet.value().tags.at(0).key);
    EXPECT_EQ("test", resultTagging.tagSet.value().tags.at(0).value);

    EXPECT_EQ(key1, resultTagging.tagSet.value().tags.at(1).key);
    EXPECT_EQ("oss-sdk", resultTagging.tagSet.value().tags.at(1).value);

    // Delete object tagging
    auto deleteTaggingOutcome =
            client->deleteObjectTagging(models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(deleteTaggingOutcome.has_value());

    // Verify tags are deleted
    Config::WaitForCacheExpire(2);
    getTaggingOutcome = client->getObjectTagging(models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(getTaggingOutcome.has_value());
    auto& resultAfterDelete = getTaggingOutcome.value();
    if (resultAfterDelete.hasTagging() && resultAfterDelete.getTagging().tagSet.has_value()) {
        EXPECT_EQ(0, resultAfterDelete.getTagging().tagSet.value().tags.size());
    }
}

TEST_F(ObjectTaggingTest, PutObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();

    models::Tagging tagging;
    auto outcome = client->putObjectTagging(
            models::PutObjectTaggingRequest().setBucket(bucketName_).setKey("test-key").setTagging(tagging));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

TEST_F(ObjectTaggingTest, GetObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome =
            client->getObjectTagging(models::GetObjectTaggingRequest().setBucket(bucketName_).setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

TEST_F(ObjectTaggingTest, DeleteObjectTagging_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto outcome =
            client->deleteObjectTagging(models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey("test-key"));
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
