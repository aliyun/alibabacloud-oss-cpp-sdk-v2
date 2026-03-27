#include <gtest/gtest.h>

#include "../Config.h"
#include "alibabacloud/oss2/OSSClient.h"
#include <random>

namespace alibabacloud {
namespace oss2 {

std::string genRandomString(size_t length) {
    static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<std::size_t> dist(0, sizeof(charset) - 2);

    std::string str(length, 0);
    std::generate_n(str.begin(), length, [&]() { return charset[dist(rng)]; });
    return str;
}


class ObjectMultipartTest : public ::testing::Test {
  protected:
    ObjectMultipartTest() {}

    ~ObjectMultipartTest() override {}

    static void SetUpTestCase() {
        auto client = Config::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.isSuccess());
    }

    static void TearDownTestCase() {
        Config::CleanBucketsByPrefix(bucketName_);
    }

    void SetUp() override {}

    void TearDown() override {}

  public:
    static std::string bucketName_;
};

std::string ObjectMultipartTest::bucketName_ = "";

// InitiateMultipartUpload Tests
TEST_F(ObjectMultipartTest, InitiateMultipartUpload_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-multipart-object";

    auto outcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(bucketName_, result.getBucket());
    EXPECT_EQ(key, result.getKey());
    EXPECT_FALSE(result.getUploadId().empty());
}

TEST_F(ObjectMultipartTest, InitiateMultipartUpload_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey("test-key"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// UploadPart and CompleteMultipartUpload Tests
TEST_F(ObjectMultipartTest, MultipartUpload_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-complete-multipart";
    std::string content1 = genRandomString(100*1024);
    std::string content2 = "Part 2 content.";

    // Initiate multipart upload
    auto initiateOutcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(initiateOutcome.isSuccess());
    std::string uploadId = initiateOutcome.getResult().getUploadId();

    // Upload part 1
    auto body1 = RequestBody::FromString(content1);

    auto part1Outcome = client->uploadPart(
        models::UploadPartRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setPartNumber(1)
            .setBody(body1));
    EXPECT_TRUE(part1Outcome.isSuccess());
    std::string etag1 = part1Outcome.getResult().getETag();
    EXPECT_FALSE(etag1.empty());

    // Upload part 2
    auto body2 = RequestBody::FromString(content2);

    auto part2Outcome = client->uploadPart(
        models::UploadPartRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setPartNumber(2)
            .setBody(body2));
    EXPECT_TRUE(part2Outcome.isSuccess());
    std::string etag2 = part2Outcome.getResult().getETag();
    EXPECT_FALSE(etag2.empty());

    // List parts
    auto listPartsOutcome = client->listParts(
        models::ListPartsRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    EXPECT_TRUE(listPartsOutcome.isSuccess());
    auto& listResult = listPartsOutcome.getResult();
    EXPECT_EQ(2, listResult.getParts().size());

    // Complete multipart upload
    std::vector<models::Part> parts;
    models::Part part1;
    part1.setPartNumber(1);
    part1.setETag(etag1);
    parts.push_back(part1);

    models::Part part2;
    part2.setPartNumber(2);
    part2.setETag(etag2);
    parts.push_back(part2);

    models::CompleteMultipartUpload completeReq;
    completeReq.setParts(parts);

    auto completeOutcome = client->completeMultipartUpload(
        models::CompleteMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setCompleteMultipartUpload(completeReq));
    EXPECT_TRUE(completeOutcome.isSuccess());
    auto& result = completeOutcome.getResult();
    EXPECT_EQ(bucketName_, result.getBucket());
    EXPECT_EQ(key, result.getKey());
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectMultipartTest, UploadPart_Fail) {
    auto client = Config::GetInvalidClient();
    std::string content = "Test content";
    auto body = RequestBody::FromString(content);


    auto outcome = client->uploadPart(
        models::UploadPartRequest()
            .setBucket(bucketName_)
            .setKey("test-key")
            .setUploadId("test-upload-id")
            .setPartNumber(1)
            .setBody(body));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

TEST_F(ObjectMultipartTest, CompleteMultipartUpload_Fail) {
    auto client = Config::GetInvalidClient();

    models::CompleteMultipartUpload completeReq;
    auto outcome = client->completeMultipartUpload(
        models::CompleteMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey("test-key")
            .setUploadId("test-upload-id")
            .setCompleteMultipartUpload(completeReq));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// AbortMultipartUpload Tests
TEST_F(ObjectMultipartTest, AbortMultipartUpload_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-abort-multipart";

    // Initiate multipart upload
    auto initiateOutcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(initiateOutcome.isSuccess());
    std::string uploadId = initiateOutcome.getResult().getUploadId();

    // Abort multipart upload
    auto outcome = client->abortMultipartUpload(
        models::AbortMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId));
    EXPECT_TRUE(outcome.isSuccess());
}

TEST_F(ObjectMultipartTest, AbortMultipartUpload_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->abortMultipartUpload(
        models::AbortMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey("test-key")
            .setUploadId("test-upload-id"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// ListMultipartUploads Tests
TEST_F(ObjectMultipartTest, ListMultipartUploads_Normal) {
    auto client = Config::GetDefaultClient();
    std::string key = "test-list-multipart-uploads";

    // Initiate a multipart upload
    auto initiateOutcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    EXPECT_TRUE(initiateOutcome.isSuccess());

    // List multipart uploads
    auto outcome = client->listMultipartUploads(
        models::ListMultipartUploadsRequest()
            .setBucket(bucketName_));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(bucketName_, result.getBucket());
    EXPECT_FALSE(result.getUploads().empty());
}

TEST_F(ObjectMultipartTest, ListMultipartUploads_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->listMultipartUploads(
        models::ListMultipartUploadsRequest()
            .setBucket(bucketName_));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// ListParts Tests
TEST_F(ObjectMultipartTest, ListParts_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->listParts(
        models::ListPartsRequest()
            .setBucket(bucketName_)
            .setKey("test-key")
            .setUploadId("test-upload-id"));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

// UploadPartCopy Tests
TEST_F(ObjectMultipartTest, UploadPartCopy_Normal) {
    auto client = Config::GetDefaultClient();
    std::string sourceKey = "test-part-copy-source";
    std::string destKey = "test-part-copy-dest";
    std::string content = "Content for part copy test. This needs to be long enough.";

    // Put source object
    auto body = RequestBody::FromString(content);

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(sourceKey)
            .setBody(body));
    EXPECT_TRUE(putOutcome.isSuccess());

    // Initiate multipart upload for destination
    auto initiateOutcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(destKey));
    EXPECT_TRUE(initiateOutcome.isSuccess());
    std::string uploadId = initiateOutcome.getResult().getUploadId();

    // Upload part copy
    auto outcome = client->uploadPartCopy(
        models::UploadPartCopyRequest()
            .setBucket(bucketName_)
            .setKey(destKey)
            .setSourceBucket(bucketName_)
            .setSourceKey(sourceKey)
            .setUploadId(uploadId)
            .setPartNumber(1));
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_FALSE(result.getETag().empty());
}

TEST_F(ObjectMultipartTest, UploadPartCopy_Fail) {
    auto client = Config::GetInvalidClient();
    auto outcome = client->uploadPartCopy(
        models::UploadPartCopyRequest()
            .setBucket(bucketName_)
            .setKey("dest-key")
            .setSourceBucket(bucketName_)
            .setSourceKey("source-key")
            .setUploadId("test-upload-id")
            .setPartNumber(1));
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}

} // namespace oss2
} // namespace alibabacloud
