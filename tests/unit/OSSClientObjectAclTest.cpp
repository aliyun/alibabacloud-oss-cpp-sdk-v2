#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"

namespace alibabacloud::oss2 {

class MockTransport : public HttpTransport {
  public:
    MockTransport() {}
    ResponseResult send(std::unique_ptr<RequestMessage>& request, RequestContext& context) override {
        return popResponse();
    }
    std::string getName() const override {
        return "MockTransport";
    }

  public:
    std::vector<std::unique_ptr<ResponseMessage>> responses;

    void Clear() {
        responses.clear();
    }

  private:
    void saveRequest(std::unique_ptr<RequestMessage>& request) {
        auto req = std::make_unique<RequestMessage>(*request);
        auto lastRequest = req.get();
        // read data
        if (lastRequest->body != nullptr) {
            auto src = lastRequest->body->spanSource();
            src->readToEnd();
        }
    }

    ResponseResult popResponse() {
        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return std::make_error_code(std::errc::result_out_of_range);
    }
};


TEST(OSSClientObjectAclTest, GetObjectAcl_FullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy>
  <Owner>
    <ID>1234</ID>
    <DisplayName>1234-desc</DisplayName>
  </Owner>
  <AccessControlList>
    <Grant>private</Grant>
  </AccessControlList>
</AccessControlPolicy>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    auto outcome = client.getObjectAcl(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasAccessControlPolicy());
    EXPECT_EQ("1234", result.getAccessControlPolicy().owner.value().id);
    EXPECT_EQ("1234-desc", result.getAccessControlPolicy().owner.value().displayName);

    EXPECT_EQ("private", result.getAccessControlPolicy().accessControlList.value().grant);
}


TEST(OSSClientObjectAclTest, GetObjectAcl_EmptyXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy>
</AccessControlPolicy>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    auto outcome = client.getObjectAcl(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasAccessControlPolicy());
    EXPECT_EQ(false, result.getAccessControlPolicy().owner.has_value());
    EXPECT_EQ(false, result.getAccessControlPolicy().accessControlList.has_value());
}


TEST(OSSClientObjectAclTest, GetObjectAcl_ErrorXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(ERROR)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    auto outcome = client.getObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ("XMLError:10", error.getCode());
    EXPECT_EQ("Error=XML_ERROR_PARSING_TEXT ErrorID=10 (0xa) Line number=1", error.getMessage());
    EXPECT_EQ("ERROR", error.getSnapshot());
    EXPECT_EQ("id-1234", error.getRequestId());
}


TEST(OSSClientObjectAclTest, GetObjectAcl_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
    <OSSAccessKeyId>ak</OSSAccessKeyId>
    <EC>0002-00000902</EC>
    <RecommendDoc>https://api.aliyun.com/troubleshoot?q=0002-00000902</RecommendDoc>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    request.setVersionId("id");
    auto outcome = client.getObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("GetObjectAcl", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?acl&versionId=id", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
    EXPECT_EQ("0002-00000902", error.getEC());
    EXPECT_EQ(7, error.getErrorFields().size());
    EXPECT_EQ("ak", error.getErrorFields().at("OSSAccessKeyId"));
    EXPECT_EQ(1, error.getHeaders().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}

TEST(OSSClientObjectAclTest, GetObjectAcl_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::GetObjectAclRequest();
    auto outcome = client.getObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("bucket");
    outcome = client.getObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());
}

TEST(OSSClientObjectAclTest, PutObjectAcl_NullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    request.setObjectAcl("public");
    auto outcome = client.putObjectAcl(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}


TEST(OSSClientObjectAclTest, PutObjectAcl_EmptyXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"()";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::PutObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    request.setObjectAcl("public");
    auto outcome = client.putObjectAcl(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientObjectAclTest, PutObjectAcl_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
    <OSSAccessKeyId>ak</OSSAccessKeyId>
    <EC>0002-00000902</EC>
    <RecommendDoc>https://api.aliyun.com/troubleshoot?q=0002-00000902</RecommendDoc>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::PutObjectAclRequest();
    request.setBucket("bucket");
    request.setKey("key");
    request.setObjectAcl("public");
    request.setVersionId("id");
    auto outcome = client.putObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("PutObjectAcl", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/key?acl&versionId=id", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
    EXPECT_EQ("The OSS Access Key Id you provided does not exist in our records.", error.getMessage());
    EXPECT_EQ("0002-00000902", error.getEC());
    EXPECT_EQ(7, error.getErrorFields().size());
    EXPECT_EQ("ak", error.getErrorFields().at("OSSAccessKeyId"));
    EXPECT_EQ(1, error.getHeaders().size());
    EXPECT_EQ("id-12345", error.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("<Error>", error.getSnapshot().substr(0, 7));
}


TEST(OSSClientObjectAclTest, PutObjectAcl_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::PutObjectAclRequest();
    auto outcome = client.putObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("bucket");
    outcome = client.putObjectAcl(request);
    EXPECT_FALSE(outcome.isSuccess());
    error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Key", error.getMessage());
}

} // namespace alibabacloud::oss2