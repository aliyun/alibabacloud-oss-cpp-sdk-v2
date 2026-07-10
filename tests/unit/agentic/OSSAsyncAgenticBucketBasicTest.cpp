#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/agentic/OSSAsyncAgenticClient.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "MockAsyncTransport.h"

#include <future>

namespace alibabacloud::oss2 {

static ClientConfiguration makeAsyncAgenticConfig(const std::shared_ptr<MockAsyncTransport>& mock,
                                                  const std::string& accountId = "1234567890") {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.accountId = accountId;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    return config;
}

TEST(OSSAsyncAgenticBucketBasicTest, CreateAgenticBucketAsync_RequiredField) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    std::promise<agentic::CreateAgenticBucketOutcome> p;
    client.createAgenticBucketAsync(agentic::models::CreateAgenticBucketRequest(),
                                    [&p](agentic::CreateAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncAgenticBucketBasicTest, CreateAgenticBucketAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket("mybucket");
    std::promise<agentic::CreateAgenticBucketOutcome> p;
    client.createAgenticBucketAsync(request, [&p](agentic::CreateAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncAgenticBucketBasicTest, GetAgenticBucketAsync_FullXml) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AgenticBucketInfo>
  <Name>mybucket</Name>
  <Region>cn-hangzhou</Region>
  <Status>enabled</Status>
</AgenticBucketInfo>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    std::promise<agentic::GetAgenticBucketOutcome> p;
    client.getAgenticBucketAsync(request, [&p](agentic::GetAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    ASSERT_TRUE(outcome.has_value());
    ASSERT_TRUE(outcome.value().hasAgenticBucketInfo());
    EXPECT_EQ("mybucket", outcome.value().getAgenticBucketInfo().name.value());
    EXPECT_EQ("enabled", outcome.value().getAgenticBucketInfo().status.value());
}

TEST(OSSAsyncAgenticBucketBasicTest, ListAgenticBucketsAsync_FullXml) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListAgenticBucketsResult>
  <Region>cn-hangzhou</Region>
  <IsTruncated>false</IsTruncated>
  <AgenticBuckets>
    <AgenticBucket>
      <Name>bucket-a</Name>
    </AgenticBucket>
  </AgenticBuckets>
</ListAgenticBucketsResult>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    std::promise<agentic::ListAgenticBucketsOutcome> p;
    client.listAgenticBucketsAsync(agentic::models::ListAgenticBucketsRequest(),
                                   [&p](agentic::ListAgenticBucketsOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, outcome.value().getAgenticBuckets().size());
    EXPECT_EQ("bucket-a", outcome.value().getAgenticBuckets()[0].name.value());
}

TEST(OSSAsyncAgenticBucketBasicTest, DeleteAgenticBucketAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = agentic::models::DeleteAgenticBucketRequest();
    request.setBucket("mybucket");
    std::promise<agentic::DeleteAgenticBucketOutcome> p;
    client.deleteAgenticBucketAsync(request, [&p](agentic::DeleteAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(204, outcome.value().getStatusCode());
}

TEST(OSSAsyncAgenticBucketBasicTest, PutAgenticBucketStatusAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = agentic::models::PutAgenticBucketStatusRequest();
    request.setBucket("mybucket");
    request.setAgenticBucketStatus(agentic::models::AgenticBucketStatus().setStatus("disabled"));
    std::promise<agentic::PutAgenticBucketStatusOutcome> p;
    client.putAgenticBucketStatusAsync(request, [&p](agentic::PutAgenticBucketStatusOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncAgenticBucketBasicTest, ListBucketSpacesAsync_FullXml) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketSpacesResult>
  <Prefix>abc</Prefix>
  <MaxKeys>10</MaxKeys>
  <IsTruncated>false</IsTruncated>
  <BucketSpaces>
    <BucketSpace>
      <Name>space-a</Name>
    </BucketSpace>
  </BucketSpaces>
</ListBucketSpacesResult>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::ListBucketSpacesRequest();
    request.setBucket("mybucket");
    std::promise<agentic::ListBucketSpacesOutcome> p;
    client.listBucketSpacesAsync(request, [&p](agentic::ListBucketSpacesOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, outcome.value().getBucketSpaces().size());
    EXPECT_EQ("space-a", outcome.value().getBucketSpaces()[0].name.value());
}

TEST(OSSAsyncAgenticBucketBasicTest, InvalidAccountId_DeferredError) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock, "bad-account"));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    std::promise<agentic::GetAgenticBucketOutcome> p;
    client.getAgenticBucketAsync(request, [&p](agentic::GetAgenticBucketOutcome o) { p.set_value(std::move(o)); });
    auto outcome = p.get_future().get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
}

// ---------------- makeAsyncBucketSpaceClient factory ----------------

TEST(OSSAsyncAgenticBucketBasicTest, MakeAsyncBucketSpaceClient_EndpointResolution) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::makeAsyncBucketSpaceClient(makeAsyncAgenticConfig(mock));

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("myspace");
    request.setKey("dir/obj.txt");
    request.setBody(std::make_shared<StringContent>("data"));
    auto outcome = client.asyncCall(request).get();
    EXPECT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    // The logical space prefix expands into the physical -bs-apsr virtual host.
    EXPECT_TRUE(req->uri.find("myspace-1234567890-cn-hangzhou-bs-apsr.oss-cn-hangzhou.aliyuncs.com") !=
                std::string::npos)
            << req->uri;
    EXPECT_TRUE(req->uri.find("dir/obj.txt") != std::string::npos) << req->uri;
}

TEST(OSSAsyncAgenticBucketBasicTest, MakeAsyncBucketSpaceClient_InvalidAccountId_DeferredError) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::makeAsyncBucketSpaceClient(makeAsyncAgenticConfig(mock, "bad-account"));

    auto request = models::PutObjectRequest();
    request.setBucket("myspace");
    request.setKey("obj.txt");
    request.setBody(std::make_shared<StringContent>("data"));
    auto outcome = client.asyncCall(request).get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    EXPECT_EQ(0, mock->requests.size());
}

// ---------------- invokeOperationAsync (raw) ----------------

TEST(OSSAsyncAgenticBucketBasicTest, InvokeOperationAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-raw"}}, nullptr}));

    OperationInput input;
    input.opName = "GetAgenticBucket";
    input.method = "GET";
    input.bucket = "mybucket";
    input.parameters.emplace("agenticBucket", "");
    std::promise<OperationResult> p;
    client.invokeOperationAsync(input, [&p](OperationResult r) { p.set_value(std::move(r)); });
    auto result = p.get_future().get();
    ASSERT_TRUE(std::holds_alternative<OperationOutput>(result));
    EXPECT_EQ(200, std::get<OperationOutput>(result).statusCode);

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    EXPECT_EQ("GET", req->method);
    EXPECT_TRUE(req->uri.find("mybucket-1234567890-cn-hangzhou-ab-apsr.oss-cn-hangzhou.aliyuncs.com") !=
                std::string::npos)
            << req->uri;
}

// ---------------- asyncCall (future) ----------------

TEST(OSSAsyncAgenticBucketBasicTest, AsyncCall_GetAgenticBucket_FullXml) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AgenticBucketInfo>
  <Name>mybucket</Name>
  <Status>enabled</Status>
</AgenticBucketInfo>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.asyncCall(request).get();
    ASSERT_TRUE(outcome.has_value());
    ASSERT_TRUE(outcome.value().hasAgenticBucketInfo());
    EXPECT_EQ("mybucket", outcome.value().getAgenticBucketInfo().name.value());
    EXPECT_EQ("enabled", outcome.value().getAgenticBucketInfo().status.value());
}

TEST(OSSAsyncAgenticBucketBasicTest, AsyncCall_ListBucketSpaces_FullXml) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketSpacesResult>
  <IsTruncated>false</IsTruncated>
  <BucketSpaces>
    <BucketSpace>
      <Name>space-a</Name>
    </BucketSpace>
  </BucketSpaces>
</ListBucketSpacesResult>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::ListBucketSpacesRequest();
    request.setBucket("mybucket");
    auto outcome = client.asyncCall(request).get();
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, outcome.value().getBucketSpaces().size());
    EXPECT_EQ("space-a", outcome.value().getBucketSpaces()[0].name.value());
}

TEST(OSSAsyncAgenticBucketBasicTest, AsyncCall_RequiredField) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = agentic::OSSAsyncAgenticBucketClient(makeAsyncAgenticConfig(mock));

    auto outcome = client.asyncCall(agentic::models::GetAgenticBucketRequest()).get();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

} // namespace alibabacloud::oss2
