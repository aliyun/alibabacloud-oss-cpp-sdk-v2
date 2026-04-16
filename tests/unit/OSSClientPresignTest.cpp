#include <gtest/gtest.h>

#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/models/Presign.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/models/ObjectMultipart.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "src/internal/ClientImpl.h"

#include <chrono>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

class MockTransportForPresign : public HttpTransport {
  public:
    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override {
        lastRequest = std::move(request);
        auto response = std::make_unique<ResponseMessage>();
        response->statusCode = 200;
        response->reason = "OK";
        response->headers = {{"x-oss-request-id", "test-request-id"}};
        response->body = std::make_shared<std::stringstream>("");
        return ResponseResult(std::move(response));
    }
    std::string getName() const override {
        return "MockTransportForPresign";
    }

    std::unique_ptr<RequestMessage> lastRequest;
};

class OSSClientPresignTest : public ::testing::Test {
  protected:
    void SetUp() override {
        mockTransport = std::make_shared<MockTransportForPresign>();
    }

    void TearDown() override {
        mockTransport.reset();
    }

    std::shared_ptr<MockTransportForPresign> mockTransport;
};

TEST_F(OSSClientPresignTest, presignPutObject) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::PutObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("PUT", result.getMethod());
    EXPECT_NE(0, result.getExpiration());
    EXPECT_GT(result.getExpiration(), std::time(nullptr));

    EXPECT_NE(std::string::npos, result.getUrl().find("test-bucket.oss-cn-hangzhou.aliyuncs.com"));
    EXPECT_NE(std::string::npos, result.getUrl().find("test-key"));
    EXPECT_NE(std::string::npos, result.getUrl().find("x-oss-signature-version=OSS4-HMAC-SHA256"));
}

TEST_F(OSSClientPresignTest, presignPutObject_noOptions) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::PutObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    std::time_t beforeCall = std::time(nullptr);
    auto outcome = client.presign(request, nullptr);
    std::time_t afterCall = std::time(nullptr);

    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("PUT", result.getMethod());

    // Verify default expiration is 15 minutes (900 seconds) from now, with +/- 2 seconds tolerance
    std::time_t expectedExpiration = beforeCall + 900;
    EXPECT_GE(result.getExpiration(), expectedExpiration - 2);
    EXPECT_LE(result.getExpiration(), expectedExpiration + 2 + (afterCall - beforeCall));
}

TEST_F(OSSClientPresignTest, presignGetObject) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::PresignOptions options;
    std::time_t expiration = std::time(nullptr) + 1800;
    options.setExpiration(expiration);

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("GET", result.getMethod());
    EXPECT_EQ(expiration, result.getExpiration());

    EXPECT_NE(std::string::npos, result.getUrl().find("test-bucket.oss-cn-hangzhou.aliyuncs.com"));
    EXPECT_NE(std::string::npos, result.getUrl().find("test-key"));
}

TEST_F(OSSClientPresignTest, presignHeadObject) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::HeadObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(7200));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("HEAD", result.getMethod());
    EXPECT_NE(0, result.getExpiration());
}

TEST_F(OSSClientPresignTest, presignUploadPart) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::UploadPartRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");
    request.setPartNumber(1);

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("PUT", result.getMethod());
    EXPECT_NE(0, result.getExpiration());
    EXPECT_NE(std::string::npos, result.getUrl().find("uploadId=test-upload-id"));
    EXPECT_NE(std::string::npos, result.getUrl().find("partNumber=1"));
}

TEST_F(OSSClientPresignTest, presignWithToken) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk", "token");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("GET", result.getMethod());
    EXPECT_NE(std::string::npos, result.getUrl().find("x-oss-security-token"));
}

TEST_F(OSSClientPresignTest, presignWithV1Signature) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("GET", result.getMethod());
    EXPECT_NE(std::string::npos, result.getUrl().find("OSSAccessKeyId="));
    EXPECT_NE(std::string::npos, result.getUrl().find("Expires="));
    EXPECT_NE(std::string::npos, result.getUrl().find("Signature="));
}

TEST_F(OSSClientPresignTest, presignEmptyCredentials) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("", "");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::CREDENTIALS_EMPTYNULL, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignGetObject_withSignedHeaders) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("Content-Type", "application/json");
    request.addHeader("Content-MD5", "d41d8cd98f00b204e9800998ecf8427e");
    request.addHeader("x-oss-meta-user", "test-user");
    request.addHeader("x-oss-meta-app", "test-app");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("GET", result.getMethod());
    EXPECT_NE(0, result.getExpiration());

    // Verify signed headers
    const auto& signedHeaders = result.getSignedHeaders();
    EXPECT_EQ(4u, signedHeaders.size());
    EXPECT_EQ("application/json", signedHeaders.at("Content-Type"));
    EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", signedHeaders.at("Content-MD5"));
    EXPECT_EQ("test-user", signedHeaders.at("x-oss-meta-user"));
    EXPECT_EQ("test-app", signedHeaders.at("x-oss-meta-app"));
}

TEST_F(OSSClientPresignTest, presignUploadPart_withSignedHeaders) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::UploadPartRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");
    request.setPartNumber(1);
    request.addHeader("Content-MD5", "1234567890abcdef1234567890abcdef");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("PUT", result.getMethod());

    // Verify signed headers
    const auto& signedHeaders = result.getSignedHeaders();
    EXPECT_GE(signedHeaders.size(), 1u);
    EXPECT_EQ("1234567890abcdef1234567890abcdef", signedHeaders.at("Content-MD5"));
}

TEST_F(OSSClientPresignTest, presignWithV1Signature_checkSignedHeaders) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.signatureVersion = "v1";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("Content-Type", "image/png");
    request.addHeader("Content-MD5", "md5-hash");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("GET", result.getMethod());

    // Verify signed headers for V1 signature
    const auto& signedHeaders = result.getSignedHeaders();
    EXPECT_EQ(2u, signedHeaders.size());
    EXPECT_EQ("image/png", signedHeaders.at("Content-Type"));
    EXPECT_EQ("md5-hash", signedHeaders.at("Content-MD5"));

    // Verify V1 signature URL parameters
    EXPECT_NE(std::string::npos, result.getUrl().find("OSSAccessKeyId=ak"));
    EXPECT_NE(std::string::npos, result.getUrl().find("Expires="));
    EXPECT_NE(std::string::npos, result.getUrl().find("Signature="));
}

// Tests for required field validation

TEST_F(OSSClientPresignTest, presignPutObject_withAdditionalHeaders) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    // Configure additionalHeaders to be signed (non x-oss-meta- headers)
    config.additionalHeaders = {"user-key1", "user-key2"};

    OSSClient client(config);

    models::PutObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.addHeader("user-key1", "user-value1");
    request.addHeader("user-key2", "user-value2");
    request.addHeader("x-oss-meta-custom", "meta-value");
    request.addHeader("Content-Type", "text/plain");

    models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(request, &options);
    ASSERT_TRUE(outcome.isSuccess());

    const auto& result = outcome.getResult();
    EXPECT_FALSE(result.getUrl().empty());
    EXPECT_EQ("PUT", result.getMethod());
    EXPECT_NE(0, result.getExpiration());

    // Verify signed headers include the additional headers
    const auto& signedHeaders = result.getSignedHeaders();
    EXPECT_GE(signedHeaders.size(), 4u);
    EXPECT_EQ("text/plain", signedHeaders.at("Content-Type"));
    EXPECT_EQ("user-value1", signedHeaders.at("user-key1"));
    EXPECT_EQ("user-value2", signedHeaders.at("user-key2"));
    EXPECT_EQ("meta-value", signedHeaders.at("x-oss-meta-custom"));
}

TEST_F(OSSClientPresignTest, presignPutObject_emptyBucket) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::PutObjectRequest request;
    request.setBucket("");
    request.setKey("test-key");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
    EXPECT_NE(std::string::npos, outcome.getError().getMessage().find("Bucket"));
}

TEST_F(OSSClientPresignTest, presignPutObject_emptyKey) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::PutObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
    EXPECT_NE(std::string::npos, outcome.getError().getMessage().find("Key"));
}

TEST_F(OSSClientPresignTest, presignGetObject_emptyBucket) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("");
    request.setKey("test-key");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignGetObject_emptyKey) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::GetObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignHeadObject_emptyBucket) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::HeadObjectRequest request;
    request.setBucket("");
    request.setKey("test-key");

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignUploadPart_emptyBucket) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::UploadPartRequest request;
    request.setBucket("");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");
    request.setPartNumber(1);

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignUploadPart_emptyUploadId) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::UploadPartRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("");
    request.setPartNumber(1);

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

TEST_F(OSSClientPresignTest, presignUploadPart_invalidPartNumber) {
    ClientConfiguration config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    config.httpTransport = mockTransport;

    OSSClient client(config);

    models::UploadPartRequest request;
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");
    request.setPartNumber(-1);

    auto outcome = client.presign(request, nullptr);
    ASSERT_FALSE(outcome.isSuccess());
    EXPECT_EQ(SdkErrorCode::ARGUMENT_REQUIRED, outcome.getError().getErrorCode());
}

} // namespace oss2
} // namespace alibabacloud
