#include <gtest/gtest.h>
#include <random>
#include <algorithm>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

static std::string genRandomString(size_t length) {
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

class AsyncObjectMultipartTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncObjectMultipartTest::bucketName_ = "";

TEST_F(AsyncObjectMultipartTest, InitiateMultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-multipart-object";

    auto future = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(bucketName_, result.getBucket());
    EXPECT_EQ(key, result.getKey());
    EXPECT_FALSE(result.getUploadId().empty());
}

TEST_F(AsyncObjectMultipartTest, InitiateMultipartUpload_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectMultipartTest, MultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-complete-multipart";
    std::string content1 = genRandomString(100 * 1024);
    std::string content2 = "Part 2 content.";

    auto initFuture = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    std::string uploadId = initOutcome.value().getUploadId();

    auto part1Future = client->asyncCall(models::UploadPartRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setPartNumber(1)
            .setBody(RequestBody::FromString(content1)));
    auto part1Outcome = part1Future.get();
    EXPECT_TRUE(part1Outcome.has_value());
    std::string etag1 = part1Outcome.value().getETag();

    auto part2Future = client->asyncCall(models::UploadPartRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setPartNumber(2)
            .setBody(RequestBody::FromString(content2)));
    auto part2Outcome = part2Future.get();
    EXPECT_TRUE(part2Outcome.has_value());
    std::string etag2 = part2Outcome.value().getETag();

    auto listPartsFuture = client->asyncCall(models::ListPartsRequest().setBucket(bucketName_).setKey(key).setUploadId(uploadId));
    auto listPartsOutcome = listPartsFuture.get();
    EXPECT_TRUE(listPartsOutcome.has_value());
    EXPECT_EQ(2, listPartsOutcome.value().getParts().size());

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

    auto completeFuture = client->asyncCall(models::CompleteMultipartUploadRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setCompleteMultipartUpload(completeReq));
    auto completeOutcome = completeFuture.get();
    EXPECT_TRUE(completeOutcome.has_value());
    EXPECT_EQ(bucketName_, completeOutcome.value().getBucket());
    EXPECT_EQ(key, completeOutcome.value().getKey());
    EXPECT_FALSE(completeOutcome.value().getETag().empty());
}

TEST_F(AsyncObjectMultipartTest, AbortMultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-abort-multipart";

    auto initFuture = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    std::string uploadId = initOutcome.value().getUploadId();

    auto future = client->asyncCall(models::AbortMultipartUploadRequest().setBucket(bucketName_).setKey(key).setUploadId(uploadId));
    EXPECT_TRUE(future.get().has_value());
}

TEST_F(AsyncObjectMultipartTest, AbortMultipartUpload_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::AbortMultipartUploadRequest().setBucket(bucketName_).setKey("test-key").setUploadId("test-upload-id"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectMultipartTest, ListMultipartUploads_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto initFuture = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey("test-list-uploads"));
    EXPECT_TRUE(initFuture.get().has_value());

    auto future = client->asyncCall(models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(bucketName_, outcome.value().getBucket());
    EXPECT_FALSE(outcome.value().getUploads().empty());
}

TEST_F(AsyncObjectMultipartTest, ListMultipartUploads_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

TEST_F(AsyncObjectMultipartTest, UploadPartCopy_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string sourceKey = "test-part-copy-source";
    std::string destKey = "test-part-copy-dest";

    auto putFuture = client->asyncCall(models::PutObjectRequest().setBucket(bucketName_).setKey(sourceKey)
            .setBody(RequestBody::FromString("Content for part copy test.")));
    EXPECT_TRUE(putFuture.get().has_value());

    auto initFuture = client->asyncCall(models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(destKey));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    std::string uploadId = initOutcome.value().getUploadId();

    auto future = client->asyncCall(models::UploadPartCopyRequest()
            .setBucket(bucketName_).setKey(destKey)
            .setSourceBucket(bucketName_).setSourceKey(sourceKey)
            .setUploadId(uploadId).setPartNumber(1));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value().getETag().empty());
}

TEST_F(AsyncObjectMultipartTest, UploadPartCopy_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->asyncCall(models::UploadPartCopyRequest()
            .setBucket(bucketName_).setKey("dest")
            .setSourceBucket(bucketName_).setSourceKey("src")
            .setUploadId("test-upload-id").setPartNumber(1));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

// --- CRC64 Upload Check Integration Tests ---

TEST_F(AsyncObjectMultipartTest, UploadPart_CRC64CheckUpload) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-uploadpart-crc64-async";

    auto initFuture = client->asyncCall(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.has_value());
    auto uploadId = initOutcome.value().getUploadId();

    std::string partData = genRandomString(100 * 1024);
    auto partFuture = client->asyncCall(
        models::UploadPartRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setPartNumber(1)
            .setBody(RequestBody::FromString(partData)));
    auto partOutcome = partFuture.get();
    EXPECT_TRUE(partOutcome.has_value());
    EXPECT_FALSE(partOutcome.value().getHashCrc64ecma().empty());

    client->asyncCall(
        models::AbortMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)).get();
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
