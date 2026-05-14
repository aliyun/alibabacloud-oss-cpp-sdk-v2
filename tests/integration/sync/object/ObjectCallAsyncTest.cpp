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
        EXPECT_TRUE(outcome.isSuccess());
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

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    auto putOutcome = putFuture.get();
    EXPECT_TRUE(putOutcome.isSuccess());

    auto getFuture = client_->callAsync<GetObjectOutcome>(
        &OSSClient::getObject,
        models::GetObjectRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getFuture.get();
    EXPECT_TRUE(getOutcome.isSuccess());
    EXPECT_EQ(content.size(), getOutcome.getResult().getContentLength());
}

// PutObject Callback
TEST_F(ObjectCallAsyncTest, PutObject_Callback) {
    std::string key = "async-put-callback";
    std::string content = "Hello Async Callback!";
    auto body = RequestBody::FromString(content);

    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body),
        [&promise](const OSSClient*, const models::PutObjectRequest&, const PutObjectOutcome& outcome) {
            promise.set_value(outcome);
        });

    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

// HeadObject Future
TEST_F(ObjectCallAsyncTest, HeadObject_Future) {
    std::string key = "async-head-future";
    std::string content = "Head me async!";
    auto body = RequestBody::FromString(content);

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto headFuture = client_->callAsync<HeadObjectOutcome>(
        &OSSClient::headObject,
        models::HeadObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = headFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(content.size(), outcome.getResult().getContentLength());
}

// CopyObject Future
TEST_F(ObjectCallAsyncTest, CopyObject_Future) {
    std::string srcKey = "async-copy-src";
    std::string dstKey = "async-copy-dst";
    auto body = RequestBody::FromString("Copy me async!");

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(srcKey).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto copyFuture = client_->callAsync<CopyObjectOutcome>(
        &OSSClient::copyObject,
        models::CopyObjectRequest()
            .setBucket(bucketName_)
            .setKey(dstKey)
            .setSourceBucket(bucketName_)
            .setSourceKey(srcKey));
    auto outcome = copyFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_FALSE(outcome.getResult().getETag().empty());
}

// DeleteObject Future
TEST_F(ObjectCallAsyncTest, DeleteObject_Future) {
    std::string key = "async-delete-future";
    auto body = RequestBody::FromString("Delete me async!");

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto delFuture = client_->callAsync<DeleteObjectOutcome>(
        &OSSClient::deleteObject,
        models::DeleteObjectRequest().setBucket(bucketName_).setKey(key));
    auto outcome = delFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
}

// AppendObject Future
TEST_F(ObjectCallAsyncTest, AppendObject_Future) {
    std::string key = "async-append-future";
    std::string content = "Append me async!";
    auto body = RequestBody::FromString(content);

    auto future = client_->callAsync<AppendObjectOutcome>(
        &OSSClient::appendObject,
        models::AppendObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setPosition(0)
            .setBody(body));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

// PutObjectAcl + GetObjectAcl Future
TEST_F(ObjectCallAsyncTest, ObjectAcl_Future) {
    std::string key = "async-acl-future";
    auto body = RequestBody::FromString("Acl test async!");

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto putAclFuture = client_->callAsync<PutObjectAclOutcome>(
        &OSSClient::putObjectAcl,
        models::PutObjectAclRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setObjectAcl("private"));
    EXPECT_TRUE(putAclFuture.get().isSuccess());

    auto getAclFuture = client_->callAsync<GetObjectAclOutcome>(
        &OSSClient::getObjectAcl,
        models::GetObjectAclRequest().setBucket(bucketName_).setKey(key));
    auto outcome = getAclFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ("private", outcome.getResult().getAccessControlPolicy().accessControlList.value().grant);
}

// PutSymlink + GetSymlink Future
TEST_F(ObjectCallAsyncTest, Symlink_Future) {
    std::string targetKey = "async-symlink-target";
    std::string symlinkKey = "async-symlink-link";
    auto body = RequestBody::FromString("Symlink target!");

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(targetKey).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto putSymFuture = client_->callAsync<PutSymlinkOutcome>(
        &OSSClient::putSymlink,
        models::PutSymlinkRequest()
            .setBucket(bucketName_)
            .setKey(symlinkKey)
            .setSymlinkTarget(targetKey));
    EXPECT_TRUE(putSymFuture.get().isSuccess());

    auto getSymFuture = client_->callAsync<GetSymlinkOutcome>(
        &OSSClient::getSymlink,
        models::GetSymlinkRequest().setBucket(bucketName_).setKey(symlinkKey));
    auto outcome = getSymFuture.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(targetKey, outcome.getResult().getSymlinkTarget());
}

// PutObjectTagging + GetObjectTagging + DeleteObjectTagging Future
TEST_F(ObjectCallAsyncTest, Tagging_Future) {
    std::string key = "async-tagging-future";
    auto body = RequestBody::FromString("Tagging test!");

    auto putFuture = client_->callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey(key).setBody(body));
    EXPECT_TRUE(putFuture.get().isSuccess());

    models::Tag tag;
    tag.key = "env";
    tag.value = "test";
    models::TagSet tagSet;
    tagSet.tags.push_back(tag);
    models::Tagging tagging;
    tagging.tagSet = tagSet;

    auto putTagFuture = client_->callAsync<PutObjectTaggingOutcome>(
        &OSSClient::putObjectTagging,
        models::PutObjectTaggingRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setTagging(tagging));
    EXPECT_TRUE(putTagFuture.get().isSuccess());

    auto getTagFuture = client_->callAsync<GetObjectTaggingOutcome>(
        &OSSClient::getObjectTagging,
        models::GetObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    auto getOutcome = getTagFuture.get();
    EXPECT_TRUE(getOutcome.isSuccess());

    auto delTagFuture = client_->callAsync<DeleteObjectTaggingOutcome>(
        &OSSClient::deleteObjectTagging,
        models::DeleteObjectTaggingRequest().setBucket(bucketName_).setKey(key));
    EXPECT_TRUE(delTagFuture.get().isSuccess());
}

// Multipart Upload Future
TEST_F(ObjectCallAsyncTest, MultipartUpload_Future) {
    std::string key = "async-multipart-future";

    auto initFuture = client_->callAsync<InitiateMultipartUploadOutcome>(
        &OSSClient::initiateMultipartUpload,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.isSuccess());
    auto uploadId = initOutcome.getResult().getUploadId();
    EXPECT_FALSE(uploadId.empty());

    auto listPartsFuture = client_->callAsync<ListPartsOutcome>(
        &OSSClient::listParts,
        models::ListPartsRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    auto listPartsOutcome = listPartsFuture.get();
    EXPECT_TRUE(listPartsOutcome.isSuccess());

    auto abortFuture = client_->callAsync<AbortMultipartUploadOutcome>(
        &OSSClient::abortMultipartUpload,
        models::AbortMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    EXPECT_TRUE(abortFuture.get().isSuccess());
}

// ListMultipartUploads Future
TEST_F(ObjectCallAsyncTest, ListMultipartUploads_Future) {
    auto future = client_->callAsync<ListMultipartUploadsOutcome>(
        &OSSClient::listMultipartUploads,
        models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
}

// No executor error
TEST_F(ObjectCallAsyncTest, NoExecutor_Error) {
    auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
    auto config = ClientConfiguration::loadDefault();
    config.region = Config::Region;
    config.endpoint = Config::Endpoint;
    config.credentialsProvider = provider;
    auto clientNoExec = OSSClient(config);

    auto future = clientNoExec.callAsync<PutObjectOutcome>(
        &OSSClient::putObject,
        models::PutObjectRequest().setBucket(bucketName_).setKey("no-exec-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("NoExecutor", outcome.getError().getCode());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
