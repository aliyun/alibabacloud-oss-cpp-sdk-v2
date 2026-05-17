#include <gtest/gtest.h>

#include "alibabacloud/oss2/Config.h"
#include "Config.h"
#include "sync/ClientHelper.h"
#include "HttpClients.h"
#include "alibabacloud/oss2/OSSClient.h"

#ifdef ALIBABACLOUD_OSS_HAS_CURL

namespace alibabacloud {
namespace oss2 {
namespace sync {


class ObjectPresignTest : public ::testing::Test {
  protected:
    ObjectPresignTest() {}

    ~ObjectPresignTest() override {}

    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(
            models::PutBucketRequest().setBucket(bucketName_));
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

std::string ObjectPresignTest::bucketName_ = "";

// Test presign GET and PUT object with only body
TEST_F(ObjectPresignTest, PresignGetAndPutObjectOnlyBody) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "root/sub-folder/1+2.bin";
    std::string content = "hello world";

    // Put object using presigned URL
    auto presignResult = client->presign(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(presignResult.has_value());
    std::string putUrl = presignResult.value().getUrl();
    EXPECT_NE(putUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_NE(putUrl.find("x-oss-signature-version=OSS4-HMAC-SHA256"), std::string::npos);
    EXPECT_EQ("PUT", presignResult.value().getMethod());

    test::HttpClient httpClient;
    auto putResponse = httpClient.put(putUrl, content);
    EXPECT_EQ(200, putResponse.statusCode);

    // Get object using presigned URL
    presignResult = client->presign(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(presignResult.has_value());
    std::string getUrl = presignResult.value().getUrl();
    EXPECT_NE(getUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_NE(getUrl.find("x-oss-signature-version=OSS4-HMAC-SHA256"), std::string::npos);
    EXPECT_EQ("GET", presignResult.value().getMethod());

    auto getResponse = httpClient.get(getUrl);
    EXPECT_EQ(200, getResponse.statusCode);
    EXPECT_EQ(content, getResponse.body);

    // Head object using presigned URL
    presignResult = client->presign(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(presignResult.has_value());
    std::string headUrl = presignResult.value().getUrl();
    EXPECT_NE(headUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_NE(headUrl.find("x-oss-signature-version=OSS4-HMAC-SHA256"), std::string::npos);
    EXPECT_EQ("HEAD", presignResult.value().getMethod());
}

// Test presign GET and PUT object with full properties
TEST_F(ObjectPresignTest, PresignGetAndPutObjectFullProps) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "root/sub-folder/full /1+2.bin";
    std::string content = "hello world";

    // Put object with full properties using presigned URL
    models::PutObjectRequest putRequest;
    putRequest.setBucket(bucketName_);
    putRequest.setKey(key);
    putRequest.setStorageClass("IA");
    putRequest.setObjectAcl("private");
    putRequest.addHeader("Content-Disposition", "1.txt");
    putRequest.addHeader("Cache-Control", "no-cache");
    putRequest.addHeader("Content-Encoding", "deflate");
    putRequest.addHeader("Expires", "Wed, 21 Oct 2015 07:28:00 GMT");
    putRequest.addHeader("Content-Type", "text/txt");
    putRequest.addHeader("Content-MD5", "XrY7u+Ae7tCTyyK7j1rNww==");
    putRequest.addHeader("x-oss-meta-key1", "value1");
    putRequest.addHeader("x-oss-meta-key2", "value2");
    putRequest.setBody(RequestBody::FromString(content));

    auto presignResult = client->presign(putRequest);

    ASSERT_TRUE(presignResult.has_value());
    std::string putUrl = presignResult.value().getUrl();
    EXPECT_NE(putUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_NE(putUrl.find("x-oss-signature-version=OSS4-HMAC-SHA256"), std::string::npos);
    EXPECT_EQ("PUT", presignResult.value().getMethod());

    // Verify signed headers are present
    const auto& signedHeaders = presignResult.value().getSignedHeaders();
    EXPECT_GE(signedHeaders.size(), 1);

    // Convert HeaderCollection to std::map<std::string, std::string>
    std::map<std::string, std::string> headerMap(signedHeaders.begin(), signedHeaders.end());

    test::HttpClient httpClient;
    auto putResponse = httpClient.put(putUrl, content, headerMap);
    EXPECT_EQ(200, putResponse.statusCode);

    // Get object and verify properties
    presignResult = client->presign(
        models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(presignResult.has_value());
    std::string getUrl = presignResult.value().getUrl();
    EXPECT_NE(getUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_EQ("GET", presignResult.value().getMethod());

    auto getResponse = httpClient.get(getUrl);
    EXPECT_EQ(200, getResponse.statusCode);
    EXPECT_EQ(content, getResponse.body);

    // Head object and verify properties
    presignResult = client->presign(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(presignResult.has_value());
    std::string headUrl = presignResult.value().getUrl();
    EXPECT_NE(headUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_EQ("HEAD", presignResult.value().getMethod());
}

// Test presign UploadPart
TEST_F(ObjectPresignTest, PresignUploadPart) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "upload-part-test/1+2.bin";
    std::string content = "hello world";

    // Initiate multipart upload to get real uploadId
    auto initiateOutcome = client->initiateMultipartUpload(
        models::InitiateMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key));
    ASSERT_TRUE(initiateOutcome.has_value());
    std::string uploadId = initiateOutcome.value().getUploadId();

    // Upload part using presigned URL
    auto presignResult = client->presign(
        models::UploadPartRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setPartNumber(1));

    ASSERT_TRUE(presignResult.has_value());
    std::string uploadUrl = presignResult.value().getUrl();
    EXPECT_NE(uploadUrl.find("uploadId="), std::string::npos);
    EXPECT_NE(uploadUrl.find("x-oss-expires"), std::string::npos);
    EXPECT_NE(uploadUrl.find("x-oss-signature-version=OSS4-HMAC-SHA256"), std::string::npos);
    EXPECT_EQ("PUT", presignResult.value().getMethod());

    test::HttpClient httpClient;
    auto uploadResponse = httpClient.put(uploadUrl, content);
    EXPECT_EQ(200, uploadResponse.statusCode);

    // Complete multipart upload
    auto completeOutcome = client->completeMultipartUpload(
        models::CompleteMultipartUploadRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setUploadId(uploadId)
            .setCompleteAll("yes"));
    EXPECT_TRUE(completeOutcome.has_value());

    auto getOutcome = client->getObject(models::GetObjectRequest()
            .setBucket(bucketName_)
            .setKey(key));

    ASSERT_TRUE(getOutcome.has_value());
    auto& result = getOutcome.value();
    EXPECT_EQ(content.size(), result.getContentLength());
    EXPECT_FALSE(result.getETag().empty());            
}


} // namespace sync
} // namespace oss2
} // namespace alibabacloud

#endif // ALIBABACLOUD_OSS_HAS_CURL
