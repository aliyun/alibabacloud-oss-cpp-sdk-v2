#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/utils/DefaultExecutor.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectCallAsyncTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        config.executor = std::make_shared<DefaultExecutor>();
        client_ = std::make_shared<OSSClient>(config);

        bucketName_ = Config::GenBucketName();
        auto outcome = client_->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::shared_ptr<OSSClient> client_;
    static std::string bucketName_;
};

std::shared_ptr<OSSClient> ObjectCallAsyncTest::client_ = nullptr;
std::string ObjectCallAsyncTest::bucketName_ = "";

// PutObject + GetObject Future
TEST_F(ObjectCallAsyncTest, PutGetObject_Future) {
    std::string key = "async-put-get-future";
    std::string content = "Hello Async Future!";
    auto body = RequestBody::FromString(content);

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    auto putOutcome = putFuture.get();
    EXPECT_TRUE(putOutcome.has_value());

    auto getFuture = client_->asyncCall(models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getFuture.get();
    EXPECT_TRUE(getOutcome.has_value());
    EXPECT_EQ(content.size(), getOutcome.value().getContentLength());
}

// PutObject Callback
TEST_F(ObjectCallAsyncTest, PutObject_Callback) {
    std::string key = "async-put-callback";
    std::string content = "Hello Async Callback!";
    auto body = RequestBody::FromString(content);

    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    client_->asyncCallback(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body),
        [&promise](const OSSClient*, const models::PutObjectRequest&, const PutObjectOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

// HeadObject Future
TEST_F(ObjectCallAsyncTest, HeadObject_Future) {
    std::string key = "async-head-future";
    std::string content = "Head me async!";
    auto body = RequestBody::FromString(content);

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    auto headFuture = client_->asyncCall(models::HeadObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = headFuture.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), outcome.value().getContentLength());
}

// CopyObject Future
TEST_F(ObjectCallAsyncTest, CopyObject_Future) {
    std::string srcKey = "async-copy-src";
    std::string dstKey = "async-copy-dst";
    auto body = RequestBody::FromString("Copy me async!");

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(srcKey).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    auto copyFuture = client_->asyncCall(models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey(dstKey)
            .setSourceBucket(bucketName_)
            .setSourceKey(srcKey));
    auto outcome = copyFuture.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value().getETag().empty());
}

// DeleteObject Future
TEST_F(ObjectCallAsyncTest, DeleteObject_Future) {
    std::string key = "async-delete-future";
    auto body = RequestBody::FromString("Delete me async!");

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    auto delFuture = client_->asyncCall(models::DeleteObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = delFuture.get();
    EXPECT_TRUE(outcome.has_value());
}

// AppendObject Future
TEST_F(ObjectCallAsyncTest, AppendObject_Future) {
    std::string key = "async-append-future";
    std::string content = "Append me async!";
    auto body = RequestBody::FromString(content);

    auto future = client_->asyncCall(models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(0)
            .setBody(body));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

// PutObjectAcl + GetObjectAcl Future
TEST_F(ObjectCallAsyncTest, ObjectAcl_Future) {
    std::string key = "async-acl-future";
    auto body = RequestBody::FromString("Acl test async!");

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    auto putAclFuture = client_->asyncCall(models::PutObjectAclRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setObjectAcl("private"));
    EXPECT_TRUE(putAclFuture.get().has_value());

    auto getAclFuture = client_->asyncCall(models::GetObjectAclRequest().setBucket(bucketName_).setKey(key));
    auto outcome = getAclFuture.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ("private", outcome.value().getAccessControlPolicy().accessControlList.value().grant);
}

// PutSymlink + GetSymlink Future
TEST_F(ObjectCallAsyncTest, Symlink_Future) {
    std::string targetKey = "async-symlink-target";
    std::string symlinkKey = "async-symlink-link";
    auto body = RequestBody::FromString("Symlink target!");

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(targetKey).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    auto putSymFuture = client_->asyncCall(models::PutSymlinkRequest()
            .setBucket(bucketName_)
            .setKey(symlinkKey)
            .setSymlinkTarget(targetKey));
    EXPECT_TRUE(putSymFuture.get().has_value());

    auto getSymFuture = client_->asyncCall(models::GetSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey));
    auto outcome = getSymFuture.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(targetKey, outcome.value().getSymlinkTarget());
}

// PutObjectTagging + GetObjectTagging + DeleteObjectTagging Future
TEST_F(ObjectCallAsyncTest, Tagging_Future) {
    std::string key = "async-tagging-future";
    auto body = RequestBody::FromString("Tagging test!");

    auto putFuture = client_->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().has_value());

    models::Tag tag;
    tag.key = "env";
    tag.value = "test";
    models::TagSet tagSet;
    tagSet.tags.push_back(tag);
    models::Tagging tagging;
    tagging.tagSet = tagSet;

    auto putTagFuture = client_->asyncCall(models::PutObjectTaggingRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setTagging(tagging));
    EXPECT_TRUE(putTagFuture.get().has_value());

    auto getTagFuture = client_->asyncCall(models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getTagFuture.get();
    EXPECT_TRUE(getOutcome.has_value());

    auto delTagFuture = client_->asyncCall(models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(delTagFuture.get().has_value());
}

// Multipart Upload Future
TEST_F(ObjectCallAsyncTest, MultipartUpload_Future) {
    std::string key = "async-multipart-future";

    auto initFuture = client_->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    auto uploadId = initOutcome.value().getUploadId();
    EXPECT_FALSE(uploadId.empty());

    auto listPartsFuture = client_->asyncCall(models::ListPartsRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    auto listPartsOutcome = listPartsFuture.get();
    EXPECT_TRUE(listPartsOutcome.has_value());

    auto abortFuture = client_->asyncCall(models::AbortMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    EXPECT_TRUE(abortFuture.get().has_value());
}

// ListMultipartUploads Future
TEST_F(ObjectCallAsyncTest, ListMultipartUploads_Future) {
    auto future = client_->asyncCall(models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

// No executor error
TEST_F(ObjectCallAsyncTest, NoExecutor_Error) {
    auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;
    auto clientNoExec = OSSClient(config);

    auto future = clientNoExec.asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey("no-exec-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoExecutor", outcome.error().getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
