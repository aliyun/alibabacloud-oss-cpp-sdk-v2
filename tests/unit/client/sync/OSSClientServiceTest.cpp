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


TEST(OSSClientServiceTest, ListBuckets_FullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListAllMyBucketsResult>
  <Owner>
    <ID>1234</ID>
    <DisplayName>1234</DisplayName>
  </Owner>
  <Buckets>
    <Bucket>
      <Comment></Comment>
      <CreationDate>2023-09-10T15:58:25.000Z</CreationDate>
      <ExtranetEndpoint>oss-cn-hangzhou.aliyuncs.com</ExtranetEndpoint>
      <IntranetEndpoint>oss-cn-hangzhou-internal.aliyuncs.com</IntranetEndpoint>
      <Location>oss-cn-hangzhou</Location>
      <Name>bucket-1</Name>
      <Region>cn-hangzhou</Region>
      <StorageClass>Standard</StorageClass>
    </Bucket>
    <Bucket>
      <Comment></Comment>
      <CreationDate>2024-09-10T16:16:46.000Z</CreationDate>
      <ExtranetEndpoint>oss-cn-hangzhou.aliyuncs.com</ExtranetEndpoint>
      <IntranetEndpoint>oss-cn-hangzhou-internal.aliyuncs.com</IntranetEndpoint>
      <Location>oss-cn-hangzhou</Location>
      <Name>bucket-2</Name>
      <Region>cn-hangzhou</Region>
      <StorageClass>IA</StorageClass>
    </Bucket>
  </Buckets>
  <Prefix>bucket-</Prefix>
  <Marker>bucket</Marker>
  <MaxKeys>2</MaxKeys>
  <IsTruncated>true</IsTruncated>
  <NextMarker>bucket-2</NextMarker>
</ListAllMyBucketsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListBucketsRequest();
    request.setMaxKeys(2);
    request.setPrefix("bucket-");
    request.setPrefix("bucket");
    auto outcome = client.listBuckets(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("bucket", result.getMarker());
    EXPECT_EQ("bucket-2", result.getNextMarker());
    EXPECT_EQ("bucket-", result.getPrefix());
    EXPECT_EQ("1234", result.getOwner().id.value_or(""));
    EXPECT_EQ("1234", result.getOwner().displayName.value_or(""));
    EXPECT_EQ(2, result.getMaxKeys());
    EXPECT_EQ(true, result.getIsTruncated());
    EXPECT_EQ(2, result.getBuckets().size());

    EXPECT_EQ("cn-hangzhou", result.getBuckets().at(0).region);
    EXPECT_EQ("2023-09-10T15:58:25.000Z", result.getBuckets().at(0).creationDate);
    EXPECT_EQ("oss-cn-hangzhou-internal.aliyuncs.com", result.getBuckets().at(0).intranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", result.getBuckets().at(0).extranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou", result.getBuckets().at(0).location);
    EXPECT_EQ("bucket-1", result.getBuckets().at(0).name);
    EXPECT_EQ("Standard", result.getBuckets().at(0).storageClass);

    EXPECT_EQ("cn-hangzhou", result.getBuckets().at(1).region);
    EXPECT_EQ("2024-09-10T16:16:46.000Z", result.getBuckets().at(1).creationDate);
    EXPECT_EQ("oss-cn-hangzhou-internal.aliyuncs.com", result.getBuckets().at(1).intranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", result.getBuckets().at(1).extranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou", result.getBuckets().at(1).location);
    EXPECT_EQ("bucket-2", result.getBuckets().at(1).name);
    EXPECT_EQ("IA", result.getBuckets().at(1).storageClass);
}


TEST(OSSClientServiceTest, ListBuckets_EmptyXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListAllMyBucketsResult>
</ListAllMyBucketsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListBucketsRequest();
    request.setMaxKeys(2);
    request.setPrefix("bucket-");
    request.setPrefix("bucket");
    auto outcome = client.listBuckets(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("", result.getMarker());
    EXPECT_EQ("", result.getNextMarker());
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getOwner().id.value_or(""));
    EXPECT_EQ("", result.getOwner().displayName.value_or(""));
    EXPECT_EQ(-1, result.getMaxKeys());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(0, result.getBuckets().size());
}


TEST(OSSClientServiceTest, ListBuckets_ErrorXml) {
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

    auto request = models::ListBucketsRequest();
    request.setMaxKeys(2);
    request.setPrefix("bucket-");
    request.setPrefix("bucket");
    auto outcome = client.listBuckets(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ("XMLError:10", error.getCode());
    EXPECT_EQ("Error=XML_ERROR_PARSING_TEXT ErrorID=10 (0xa) Line number=1", error.getMessage());
    EXPECT_EQ("ERROR", error.getSnapshot());
    EXPECT_EQ("id-1234", error.getRequestId());
}


TEST(OSSClientServiceTest, ListBuckets_ErrorResponse) {
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

    auto request = models::ListBucketsRequest();
    request.setMaxKeys(2);
    request.setPrefix("bucket-");
    request.setMarker("bucket");
    auto outcome = client.listBuckets(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListBuckets", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/?marker=bucket&max-keys=2&prefix=bucket-",
              error.getRequestTarget());
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

} // namespace alibabacloud::oss2