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

std::string AsyncObjectMultipartTest::bucketName_ = "";

TEST_F(AsyncObjectMultipartTest, InitiateMultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-multipart-object";

    auto future = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();
    EXPECT_EQ(bucketName_, result.getBucket());
    EXPECT_EQ(key, result.getKey());
    EXPECT_FALSE(result.getUploadId().empty());
}

TEST_F(AsyncObjectMultipartTest, InitiateMultipartUpload_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey("test-key"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectMultipartTest, MultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-complete-multipart";
    std::string content1 = genRandomString(100 * 1024);
    std::string content2 = "Part 2 content.";

    auto initFuture = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.isSuccess());
    std::string uploadId = initOutcome.getResult().getUploadId();

    auto part1Future = client->callAsync<UploadPartOutcome>(
        &OSSAsyncClient::uploadPartAsync,
        models::UploadPartRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setPartNumber(1)
            .setBody(RequestBody::FromString(content1)));
    auto part1Outcome = part1Future.get();
    EXPECT_TRUE(part1Outcome.isSuccess());
    std::string etag1 = part1Outcome.getResult().getETag();

    auto part2Future = client->callAsync<UploadPartOutcome>(
        &OSSAsyncClient::uploadPartAsync,
        models::UploadPartRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setPartNumber(2)
            .setBody(RequestBody::FromString(content2)));
    auto part2Outcome = part2Future.get();
    EXPECT_TRUE(part2Outcome.isSuccess());
    std::string etag2 = part2Outcome.getResult().getETag();

    auto listPartsFuture = client->callAsync<ListPartsOutcome>(
        &OSSAsyncClient::listPartsAsync,
        models::ListPartsRequest().setBucket(bucketName_).setKey(key).setUploadId(uploadId));
    auto listPartsOutcome = listPartsFuture.get();
    EXPECT_TRUE(listPartsOutcome.isSuccess());
    EXPECT_EQ(2, listPartsOutcome.getResult().getParts().size());

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

    auto completeFuture = client->callAsync<CompleteMultipartUploadOutcome>(
        &OSSAsyncClient::completeMultipartUploadAsync,
        models::CompleteMultipartUploadRequest()
            .setBucket(bucketName_).setKey(key).setUploadId(uploadId).setCompleteMultipartUpload(completeReq));
    auto completeOutcome = completeFuture.get();
    EXPECT_TRUE(completeOutcome.isSuccess());
    EXPECT_EQ(bucketName_, completeOutcome.getResult().getBucket());
    EXPECT_EQ(key, completeOutcome.getResult().getKey());
    EXPECT_FALSE(completeOutcome.getResult().getETag().empty());
}

TEST_F(AsyncObjectMultipartTest, AbortMultipartUpload_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "test-abort-multipart";

    auto initFuture = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(key));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.isSuccess());
    std::string uploadId = initOutcome.getResult().getUploadId();

    auto future = client->callAsync<AbortMultipartUploadOutcome>(
        &OSSAsyncClient::abortMultipartUploadAsync,
        models::AbortMultipartUploadRequest().setBucket(bucketName_).setKey(key).setUploadId(uploadId));
    EXPECT_TRUE(future.get().isSuccess());
}

TEST_F(AsyncObjectMultipartTest, AbortMultipartUpload_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<AbortMultipartUploadOutcome>(
        &OSSAsyncClient::abortMultipartUploadAsync,
        models::AbortMultipartUploadRequest().setBucket(bucketName_).setKey("test-key").setUploadId("test-upload-id"));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectMultipartTest, ListMultipartUploads_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    auto initFuture = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey("test-list-uploads"));
    EXPECT_TRUE(initFuture.get().isSuccess());

    auto future = client->callAsync<ListMultipartUploadsOutcome>(
        &OSSAsyncClient::listMultipartUploadsAsync,
        models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_EQ(bucketName_, outcome.getResult().getBucket());
    EXPECT_FALSE(outcome.getResult().getUploads().empty());
}

TEST_F(AsyncObjectMultipartTest, ListMultipartUploads_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<ListMultipartUploadsOutcome>(
        &OSSAsyncClient::listMultipartUploadsAsync,
        models::ListMultipartUploadsRequest().setBucket(bucketName_));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

TEST_F(AsyncObjectMultipartTest, UploadPartCopy_Normal) {
    auto client = ClientHelper::GetDefaultClient();
    std::string sourceKey = "test-part-copy-source";
    std::string destKey = "test-part-copy-dest";

    auto putFuture = client->callAsync<PutObjectOutcome>(
        &OSSAsyncClient::putObjectAsync,
        models::PutObjectRequest().setBucket(bucketName_).setKey(sourceKey)
            .setBody(RequestBody::FromString("Content for part copy test.")));
    EXPECT_TRUE(putFuture.get().isSuccess());

    auto initFuture = client->callAsync<InitiateMultipartUploadOutcome>(
        &OSSAsyncClient::initiateMultipartUploadAsync,
        models::InitiateMultipartUploadRequest().setBucket(bucketName_).setKey(destKey));
    auto initOutcome = initFuture.get();
    EXPECT_TRUE(initOutcome.isSuccess());
    std::string uploadId = initOutcome.getResult().getUploadId();

    auto future = client->callAsync<UploadPartCopyOutcome>(
        &OSSAsyncClient::uploadPartCopyAsync,
        models::UploadPartCopyRequest()
            .setBucket(bucketName_).setKey(destKey)
            .setSourceBucket(bucketName_).setSourceKey(sourceKey)
            .setUploadId(uploadId).setPartNumber(1));
    auto outcome = future.get();
    EXPECT_TRUE(outcome.isSuccess());
    EXPECT_FALSE(outcome.getResult().getETag().empty());
}

TEST_F(AsyncObjectMultipartTest, UploadPartCopy_Fail) {
    auto client = ClientHelper::GetInvalidClient();
    auto future = client->callAsync<UploadPartCopyOutcome>(
        &OSSAsyncClient::uploadPartCopyAsync,
        models::UploadPartCopyRequest()
            .setBucket(bucketName_).setKey("dest")
            .setSourceBucket(bucketName_).setSourceKey("src")
            .setUploadId("test-upload-id").setPartNumber(1));
    auto outcome = future.get();
    EXPECT_FALSE(outcome.isSuccess());
    EXPECT_EQ("InvalidAccessKeyId", outcome.getError().getCode());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
