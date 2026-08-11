#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/BucketSpaceHelper.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "src/agentic/AgenticUtils.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

static ClientConfiguration makeAgenticConfig(const std::shared_ptr<MockTransport>& mock,
                                             const std::string& accountId = "1234567890") {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.accountId = accountId;
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    return config;
}

// ---------------- CreateAgenticBucket ----------------

TEST(OSSAgenticBucketBasicTest, CreateAgenticBucket_RequiredField) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto request = agentic::models::CreateAgenticBucketRequest();
    auto outcome = client.createAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
    EXPECT_EQ("Missing field Bucket", outcome.error().getMessage());
}

TEST(OSSAgenticBucketBasicTest, CreateAgenticBucket_Success) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket("mybucket");
    request.setCreateAgenticBucketConfiguration(
            agentic::models::CreateAgenticBucketConfiguration().setStorageClass("Standard"));
    auto outcome = client.createAgenticBucket(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("id-1234", outcome.value().getRequestId());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    EXPECT_EQ("PUT", req->method);
    // Resolver expands the logical bucket into the physical -ab-apsr host.
    EXPECT_TRUE(req->uri.find("mybucket-1234567890-cn-hangzhou-ab-apsr.oss-cn-hangzhou.aliyuncs.com") !=
                std::string::npos);
    EXPECT_TRUE(req->uri.find("agenticBucket") != std::string::npos);
}

TEST(OSSAgenticBucketBasicTest, CreateAgenticBucket_ErrorResponse) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto body = R"(<Error>
    <Code>InvalidAccessKeyId</Code>
    <Message>The OSS Access Key Id you provided does not exist in our records.</Message>
    <RequestId>id-1234</RequestId>
</Error>
)";
    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            403, "Forbidden", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.createAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(403, outcome.error().getStatusCode());
    EXPECT_EQ("CreateAgenticBucket", outcome.error().getOpName());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

// ---------------- DeleteAgenticBucket ----------------

TEST(OSSAgenticBucketBasicTest, DeleteAgenticBucket_RequiredField) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto outcome = client.deleteAgenticBucket(agentic::models::DeleteAgenticBucketRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAgenticBucketBasicTest, DeleteAgenticBucket_Success) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = agentic::models::DeleteAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.deleteAgenticBucket(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(204, outcome.value().getStatusCode());

    ASSERT_EQ(1, mock->requests.size());
    EXPECT_EQ("DELETE", mock->requests[0]->method);
    EXPECT_TRUE(mock->requests[0]->uri.find("mybucket-1234567890-cn-hangzhou-ab-apsr") != std::string::npos);
}

// ---------------- GetAgenticBucket ----------------

TEST(OSSAgenticBucketBasicTest, GetAgenticBucket_FullXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<AgenticBucketInfo>
  <Name>mybucket</Name>
  <Owner>1234567890123456</Owner>
  <Region>cn-hangzhou</Region>
  <StorageClass>Standard</StorageClass>
  <DataRedundancyType>LRS</DataRedundancyType>
  <Status>Enabled</Status>
  <BucketResourceType>agentic</BucketResourceType>
  <CreateTime>2024-01-01T00:00:00.000Z</CreateTime>
  <ACL>private</ACL>
  <PublicAccessBlock>true</PublicAccessBlock>
  <ServerSideEncryptionRule>
    <ApplyServerSideEncryptionByDefault>
      <SSEAlgorithm>AES256</SSEAlgorithm>
    </ApplyServerSideEncryptionByDefault>
  </ServerSideEncryptionRule>
  <Versioning>Disabled</Versioning>
  <BucketPolicy></BucketPolicy>
</AgenticBucketInfo>
)";
    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.getAgenticBucket(request);
    ASSERT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(200, result.getStatusCode());
    ASSERT_TRUE(result.hasAgenticBucketInfo());
    auto& info = result.getAgenticBucketInfo();
    EXPECT_EQ("mybucket", info.name.value());
    EXPECT_EQ("1234567890123456", info.owner.value());
    EXPECT_EQ("cn-hangzhou", info.region.value());
    EXPECT_EQ("Standard", info.storageClass.value());
    EXPECT_EQ("LRS", info.dataRedundancyType.value());
    EXPECT_EQ("Enabled", info.status.value());
    EXPECT_EQ("agentic", info.bucketResourceType.value());
    EXPECT_EQ("2024-01-01T00:00:00.000Z", info.createTime.value());
    EXPECT_EQ("private", info.acl.value());
    EXPECT_EQ("true", info.publicAccessBlock.value());
    ASSERT_TRUE(info.serverSideEncryptionRule.has_value());
    EXPECT_EQ("AES256", info.serverSideEncryptionRule.value().sseAlgorithm.value());
    EXPECT_EQ("Disabled", info.versioning.value());
}

TEST(OSSAgenticBucketBasicTest, GetAgenticBucket_RequiredField) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto outcome = client.getAgenticBucket(agentic::models::GetAgenticBucketRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAgenticBucketBasicTest, GetAgenticBucket_ErrorXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("ERROR")}));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.getAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ERROR", outcome.error().getSnapshot());
}

// ---------------- ListAgenticBuckets ----------------

TEST(OSSAgenticBucketBasicTest, ListAgenticBuckets_FullXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListAgenticBucketsResult>
  <Region>cn-hangzhou</Region>
  <Owner>owner-123</Owner>
  <ContinuationToken></ContinuationToken>
  <NextContinuationToken>next-token</NextContinuationToken>
  <IsTruncated>true</IsTruncated>
  <AgenticBuckets>
    <AgenticBucket>
      <Name>bucket-a</Name>
      <StorageClass>Standard</StorageClass>
      <DataRedundancyType>LRS</DataRedundancyType>
      <CreateTime>2024-01-01T00:00:00.000Z</CreateTime>
    </AgenticBucket>
    <AgenticBucket>
      <Name>bucket-b</Name>
      <StorageClass>IA</StorageClass>
    </AgenticBucket>
  </AgenticBuckets>
</ListAgenticBucketsResult>
)";
    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::ListAgenticBucketsRequest();
    auto outcome = client.listAgenticBuckets(request);
    ASSERT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ("cn-hangzhou", result.getRegion().value());
    EXPECT_EQ("owner-123", result.getOwner().value());
    EXPECT_EQ("next-token", result.getNextContinuationToken().value());
    EXPECT_TRUE(result.getIsTruncated());
    ASSERT_EQ(2, result.getAgenticBuckets().size());
    EXPECT_EQ("bucket-a", result.getAgenticBuckets()[0].name.value());
    EXPECT_EQ("Standard", result.getAgenticBuckets()[0].storageClass.value());
    EXPECT_EQ("bucket-b", result.getAgenticBuckets()[1].name.value());
    EXPECT_EQ("IA", result.getAgenticBuckets()[1].storageClass.value());

    // ListAgenticBuckets is region-level: the host must be the plain regional
    // endpoint, with no -ab-apsr bucket prefix.
    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    EXPECT_TRUE(req->uri.find("oss-cn-hangzhou.aliyuncs.com") != std::string::npos);
    EXPECT_TRUE(req->uri.find("-ab-apsr") == std::string::npos);
}

TEST(OSSAgenticBucketBasicTest, ListAgenticBuckets_ErrorXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("ERROR")}));

    auto outcome = client.listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ERROR", outcome.error().getSnapshot());
}

// ---------------- PutAgenticBucketStatus ----------------

TEST(OSSAgenticBucketBasicTest, PutAgenticBucketStatus_RequiredField) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto request = agentic::models::PutAgenticBucketStatusRequest();
    request.setBucket("mybucket");
    // Missing AgenticBucketStatus body.
    auto outcome = client.putAgenticBucketStatus(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAgenticBucketBasicTest, PutAgenticBucketStatus_Success) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = agentic::models::PutAgenticBucketStatusRequest();
    request.setBucket("mybucket");
    request.setAgenticBucketStatus(agentic::models::AgenticBucketStatus().setStatus("disabled"));
    auto outcome = client.putAgenticBucketStatus(request);
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());

    ASSERT_EQ(1, mock->requests.size());
    EXPECT_EQ("PUT", mock->requests[0]->method);
    EXPECT_TRUE(mock->requests[0]->uri.find("status") != std::string::npos);
}

// ---------------- ListBucketSpaces ----------------

TEST(OSSAgenticBucketBasicTest, ListBucketSpaces_FullXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketSpacesResult>
  <Owner>
    <ID>owner-id</ID>
    <DisplayName>owner-name</DisplayName>
  </Owner>
  <Prefix>abc</Prefix>
  <MaxKeys>100</MaxKeys>
  <ContinuationToken></ContinuationToken>
  <NextContinuationToken>next-token</NextContinuationToken>
  <StartAfter>space-0</StartAfter>
  <IsTruncated>false</IsTruncated>
  <BucketSpaces>
    <BucketSpace>
      <Name>space-a</Name>
      <Location>cn-hangzhou</Location>
      <CreationDate>2024-01-01T00:00:00.000Z</CreationDate>
      <StorageClass>Standard</StorageClass>
    </BucketSpace>
  </BucketSpaces>
</ListBucketSpacesResult>
)";
    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = agentic::models::ListBucketSpacesRequest();
    request.setBucket("mybucket");
    request.setStartAfter("space-0");
    auto outcome = client.listBucketSpaces(request);
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, mock->requests.size());
    EXPECT_TRUE(mock->requests[0]->uri.find("start-after=space-0") != std::string::npos);
    auto& result = outcome.value();
    ASSERT_TRUE(result.getOwner().has_value());
    EXPECT_EQ("owner-id", result.getOwner().value().id);
    EXPECT_EQ("abc", result.getPrefix().value());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_EQ("next-token", result.getNextContinuationToken().value());
    EXPECT_EQ("space-0", result.getStartAfter().value());
    EXPECT_FALSE(result.getIsTruncated());
    ASSERT_EQ(1, result.getBucketSpaces().size());
    EXPECT_EQ("space-a", result.getBucketSpaces()[0].name.value());
    EXPECT_EQ("cn-hangzhou", result.getBucketSpaces()[0].location.value());
    EXPECT_EQ("Standard", result.getBucketSpaces()[0].storageClass.value());
}

TEST(OSSAgenticBucketBasicTest, ListBucketSpaces_RequiredField) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    auto outcome = client.listBucketSpaces(agentic::models::ListBucketSpacesRequest());
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAgenticBucketBasicTest, ListBucketSpaces_ErrorXml) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("ERROR")}));

    auto request = agentic::models::ListBucketSpacesRequest();
    request.setBucket("mybucket");
    auto outcome = client.listBucketSpaces(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ERROR", outcome.error().getSnapshot());
}

// ---------------- Deferred init error (invalid accountId) ----------------

TEST(OSSAgenticBucketBasicTest, InvalidAccountId_DeferredError) {
    auto mock = std::make_shared<MockTransport>();
    // Construction succeeds even with an invalid (non-digit) account id.
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock, "bad-account"));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.getAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    // No request should have been sent.
    EXPECT_EQ(0, mock->requests.size());
}

TEST(OSSAgenticBucketBasicTest, EmptyAccountId_Blocked) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock, ""));

    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.createAgenticBucket(request);
    // A bucket-scoped op requires a non-empty account id.
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    EXPECT_EQ(make_error_code(ClientErrorCode::AccountIdNull), outcome.error().getErrorCode());
    EXPECT_EQ(0, mock->requests.size());
}

TEST(OSSAgenticBucketBasicTest, MissingRegion_Blocked) {
    auto mock = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    // Endpoint set explicitly so the missing region does not fail earlier.
    config.endpoint = "oss-cn-hangzhou.aliyuncs.com";
    config.accountId = "1234567890";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    auto client = agentic::OSSAgenticBucketClient(config);

    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.createAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    EXPECT_EQ(make_error_code(ClientErrorCode::EndpointRegionNull), outcome.error().getErrorCode());
    EXPECT_EQ(0, mock->requests.size());
}

TEST(OSSAgenticBucketBasicTest, HostLabelWithinLimit) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    // bucket(32) + "-1234567890-cn-hangzhou-ab-apsr"(31) = 63 == max label length.
    auto bucket = std::string(32, 'a');
    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket(bucket);
    auto outcome = client.createAgenticBucket(request);
    EXPECT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    auto name = bucket + "-1234567890-cn-hangzhou-ab-apsr";
    EXPECT_EQ(63u, name.size());
    EXPECT_TRUE(req->uri.find(name + ".oss-cn-hangzhou.aliyuncs.com") != std::string::npos) << req->uri;
}

TEST(OSSAgenticBucketBasicTest, HostLabelTooLong_Blocked) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    // bucket(33) + 31 = 64 > 63 max label length.
    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket(std::string(33, 'a'));
    auto outcome = client.createAgenticBucket(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    EXPECT_EQ(make_error_code(ClientErrorCode::HostLabelTooLong), outcome.error().getErrorCode());
    EXPECT_NE(std::string::npos, outcome.error().getMessage().find("EndpointProvider returns error"))
            << outcome.error().getMessage();
    EXPECT_NE(std::string::npos, outcome.error().getMessage().find("exceeds the maximum length of 63 characters"))
            << outcome.error().getMessage();
    EXPECT_EQ(0, mock->requests.size());
}

TEST(OSSAgenticBucketBasicTest, HostLabelTooLong_PathStyleAllowed) {
    auto mock = std::make_shared<MockTransport>();
    auto config = makeAgenticConfig(mock);
    config.usePathStyle = true;
    auto client = agentic::OSSAgenticBucketClient(config);

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    // Path-style has no host-label length limit; the long physical name goes in the path.
    auto bucket = std::string(33, 'a');
    auto request = agentic::models::CreateAgenticBucketRequest();
    request.setBucket(bucket);
    auto outcome = client.createAgenticBucket(request);
    EXPECT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    auto name = bucket + "-1234567890-cn-hangzhou-ab-apsr";
    EXPECT_TRUE(req->uri.find("oss-cn-hangzhou.aliyuncs.com/" + name + "/") != std::string::npos) << req->uri;
}

TEST(OSSAgenticBucketBasicTest, MissingAccountId_ServiceLevelNotBlocked) {
    auto mock = std::make_shared<MockTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    auto client = agentic::OSSAgenticBucketClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListAgenticBucketsResult>
  <Region>cn-hangzhou</Region>
  <IsTruncated>false</IsTruncated>
</ListAgenticBucketsResult>
)";
    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto outcome = client.listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, mock->requests.size());
    EXPECT_TRUE(mock->requests[0]->uri.find("oss-cn-hangzhou.aliyuncs.com") != std::string::npos);
}

// ---------------- BucketSpaceHelper ----------------

TEST(OSSAgenticBucketBasicTest, BucketSpaceHelper_ToBucketName) {
    agentic::BucketSpaceHelper helper("1234567890", "cn-hangzhou");
    EXPECT_EQ("myspace-1234567890-cn-hangzhou-bs-apsr", helper.toBucketName("myspace"));
    EXPECT_EQ("1234567890", helper.getAccountId());
    EXPECT_EQ("cn-hangzhou", helper.getRegion());
}

// ---------------- makeBucketSpaceClient factory ----------------

TEST(OSSAgenticBucketBasicTest, MakeBucketSpaceClient_EndpointResolution) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::makeBucketSpaceClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("myspace");
    request.setKey("dir/obj.txt");
    request.setBody(std::make_shared<StringContent>("data"));
    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    // The logical space prefix expands into the physical -bs-apsr virtual host.
    EXPECT_TRUE(req->uri.find("myspace-1234567890-cn-hangzhou-bs-apsr.oss-cn-hangzhou.aliyuncs.com") !=
                std::string::npos)
            << req->uri;
    EXPECT_TRUE(req->uri.find("dir/obj.txt") != std::string::npos) << req->uri;
}

TEST(OSSAgenticBucketBasicTest, MakeBucketSpaceClient_InvalidAccountId_DeferredError) {
    auto mock = std::make_shared<MockTransport>();
    // Construction succeeds even with a non-digit account id.
    auto client = agentic::makeBucketSpaceClient(makeAgenticConfig(mock, "bad-account"));

    auto request = models::PutObjectRequest();
    request.setBucket("myspace");
    request.setKey("obj.txt");
    request.setBody(std::make_shared<StringContent>("data"));
    auto outcome = client.putObject(request);
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("IllegalArgument", outcome.error().getCode());
    EXPECT_EQ(0, mock->requests.size());
}

// ---------------- Path-style addressing ----------------

TEST(OSSAgenticBucketBasicTest, GetAgenticBucket_PathStyle) {
    auto mock = std::make_shared<MockTransport>();
    auto config = makeAgenticConfig(mock);
    config.usePathStyle = true;
    auto client = agentic::OSSAgenticBucketClient(config);

    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1"}},
            std::make_shared<std::stringstream>("<AgenticBucketInfo><Name>mybucket</Name></AgenticBucketInfo>")}));

    auto request = agentic::models::GetAgenticBucketRequest();
    request.setBucket("mybucket");
    auto outcome = client.getAgenticBucket(request);
    ASSERT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    // Under path-style the physical name lives in the path, host stays bare.
    EXPECT_TRUE(req->uri.find("oss-cn-hangzhou.aliyuncs.com/mybucket-1234567890-cn-hangzhou-ab-apsr/") !=
                std::string::npos)
            << req->uri;
    EXPECT_TRUE(req->uri.find("mybucket-1234567890-cn-hangzhou-ab-apsr.oss-cn-hangzhou.aliyuncs.com") ==
                std::string::npos)
            << req->uri;
    EXPECT_TRUE(req->uri.find("agenticBucket") != std::string::npos);
}

TEST(OSSAgenticBucketBasicTest, ListAgenticBuckets_PathStyle_RegionHost) {
    auto mock = std::make_shared<MockTransport>();
    auto config = makeAgenticConfig(mock);
    config.usePathStyle = true;
    auto client = agentic::OSSAgenticBucketClient(config);

    mock->Clear();
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            200, "OK", {{"x-oss-request-id", "id-1"}},
            std::make_shared<std::stringstream>("<ListAgenticBucketsResult><IsTruncated>false</IsTruncated></ListAgenticBucketsResult>")}));

    auto outcome = client.listAgenticBuckets(agentic::models::ListAgenticBucketsRequest());
    ASSERT_TRUE(outcome.has_value());
    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    // No bucket -> bare regional host, no -ab-apsr prefix anywhere.
    EXPECT_TRUE(req->uri.find("oss-cn-hangzhou.aliyuncs.com") != std::string::npos) << req->uri;
    EXPECT_TRUE(req->uri.find("-ab-apsr") == std::string::npos) << req->uri;
}

TEST(OSSAgenticBucketBasicTest, MakeBucketSpaceClient_PathStyle) {
    auto mock = std::make_shared<MockTransport>();
    auto config = makeAgenticConfig(mock);
    config.usePathStyle = true;
    auto client = agentic::makeBucketSpaceClient(config);

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("myspace");
    request.setKey("dir/obj.txt");
    request.setBody(std::make_shared<StringContent>("data"));
    auto outcome = client.putObject(request);
    EXPECT_TRUE(outcome.has_value());

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    EXPECT_TRUE(req->uri.find("oss-cn-hangzhou.aliyuncs.com/myspace-1234567890-cn-hangzhou-bs-apsr/dir/obj.txt") !=
                std::string::npos)
            << req->uri;
}

// ---------------- invokeOperation (raw) ----------------

TEST(OSSAgenticBucketBasicTest, InvokeOperation_Success) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock));

    mock->Clear();
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-raw"}}, nullptr}));

    OperationInput input;
    input.opName = "GetAgenticBucket";
    input.method = "GET";
    input.bucket = "mybucket";
    input.parameters.emplace("agenticBucket", "");
    auto result = client.invokeOperation(input);
    ASSERT_TRUE(std::holds_alternative<OperationOutput>(result));
    EXPECT_EQ(200, std::get<OperationOutput>(result).statusCode);

    ASSERT_EQ(1, mock->requests.size());
    auto& req = mock->requests[0];
    EXPECT_EQ("GET", req->method);
    // The raw entry point still routes through the agentic resolver.
    EXPECT_TRUE(req->uri.find("mybucket-1234567890-cn-hangzhou-ab-apsr.oss-cn-hangzhou.aliyuncs.com") !=
                std::string::npos)
            << req->uri;
}

TEST(OSSAgenticBucketBasicTest, InvokeOperation_InvalidAccountId_DeferredError) {
    auto mock = std::make_shared<MockTransport>();
    auto client = agentic::OSSAgenticBucketClient(makeAgenticConfig(mock, "bad-account"));

    OperationInput input;
    input.opName = "GetAgenticBucket";
    input.method = "GET";
    input.bucket = "mybucket";
    input.parameters.emplace("agenticBucket", "");
    auto result = client.invokeOperation(input);
    ASSERT_TRUE(std::holds_alternative<OperationError>(result));
    EXPECT_EQ("IllegalArgument", std::get<OperationError>(result).getCode());
    EXPECT_EQ(0, mock->requests.size());
}

// ---------------- Alias-style addressing ----------------

// The agentic clients resolve the addressing style from ClientConfiguration, which exposes
// no alias flag, so the alias routing is exercised through the shared option builder.
static ClientOptions makeAgenticOptions(AddressStyleType addressStyle, const std::string& suffix,
                                        const std::string& accountId = "1234567890",
                                        const std::string& region = "cn-hangzhou") {
    ClientOptions opts;
    opts.endpoint = "https://oss-cn-hangzhou.aliyuncs.com";
    opts.addressStyle = addressStyle;
    for (const auto& fn : agentic::makeAgenticOptionsFns(accountId, region, suffix)) {
        fn(opts);
    }
    return opts;
}

static OperationInput makeInput(const std::optional<std::string>& bucket,
                                const std::optional<std::string>& key = std::nullopt) {
    OperationInput input;
    input.opName = "GetAgenticBucket";
    input.method = "GET";
    input.bucket = bucket;
    input.key = key;
    return input;
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_AgenticBucketHost) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr");
    std::error_code ec;
    auto url = opts.endpointProvider(makeInput("mybucket"), ec);
    EXPECT_FALSE(ec);
    // The short alias label replaces "{accountId}-{region}" in the host.
    EXPECT_EQ("https://mybucket-alias-ab-apsr.oss-cn-hangzhou.aliyuncs.com/", url);
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_BucketSpaceHostAndKey) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-bs-apsr");
    std::error_code ec;
    auto url = opts.endpointProvider(makeInput("myspace", "dir/obj.txt"), ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ("https://myspace-alias-bs-apsr.oss-cn-hangzhou.aliyuncs.com/dir/obj.txt", url);
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_NoBucketUsesRegionHost) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr");
    std::error_code ec;
    auto url = opts.endpointProvider(makeInput(std::nullopt), ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/", url);
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_SignsWithFullName) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr");
    // The short label only shows up in the host, signing keeps the full name.
    EXPECT_EQ("mybucket-1234567890-cn-hangzhou-ab-apsr", opts.bucketNameResolver(makeInput("mybucket")));

    // so accountId / region stay required
    std::error_code ec;
    auto noAccount = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr", "");
    noAccount.endpointProvider(makeInput("mybucket"), ec);
    EXPECT_EQ(make_error_code(ClientErrorCode::AccountIdNull), ec);

    ec.clear();
    auto noRegion = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr", "1234567890", "");
    noRegion.endpointProvider(makeInput("mybucket"), ec);
    EXPECT_EQ(make_error_code(ClientErrorCode::EndpointRegionNull), ec);
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_HostLabelWithinLimit) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr");
    // bucket(49) + "-alias-ab-apsr"(14) = 63 == max label length.
    auto bucket = std::string(49, 'a');
    auto label = bucket + "-alias-ab-apsr";
    EXPECT_EQ(63u, label.size());

    std::error_code ec;
    auto url = opts.endpointProvider(makeInput(bucket), ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ("https://" + label + ".oss-cn-hangzhou.aliyuncs.com/", url);
}

TEST(OSSAgenticBucketBasicTest, AliasStyle_HostLabelTooLong) {
    auto opts = makeAgenticOptions(AddressStyleType::VirtualHostedAlias, "-ab-apsr");
    // bucket(50) + 14 = 64 > 63 max label length.
    std::error_code ec;
    opts.endpointProvider(makeInput(std::string(50, 'a')), ec);
    EXPECT_EQ(make_error_code(ClientErrorCode::HostLabelTooLong), ec);
}

} // namespace alibabacloud::oss2
