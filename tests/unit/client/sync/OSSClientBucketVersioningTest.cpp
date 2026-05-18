#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {


TEST(OSSClientBucketVersioningTest, PutBucketVersioning_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketVersioningRequest();
    request.setBucket("bucket");
    request.setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled"));
    auto outcome = client.putBucketVersioning(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(1, mockHandler->requests.size());
    auto& req = mockHandler->requests[0];
    EXPECT_EQ("PUT", req->method);
    EXPECT_TRUE(req->uri.find("?versioning") != std::string::npos);
}


TEST(OSSClientBucketVersioningTest, PutBucketVersioning_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::PutBucketVersioningRequest();
    auto outcome = client.putBucketVersioning(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());
}


TEST(OSSClientBucketVersioningTest, PutBucketVersioning_ErrorResponse) {
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

    auto request = models::PutBucketVersioningRequest();
    request.setBucket("bucket");
    request.setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled"));
    auto outcome = client.putBucketVersioning(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("PutBucketVersioning", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?versioning", error.getRequestTarget());
    EXPECT_EQ("id-1234", error.getRequestId());
    EXPECT_EQ("InvalidAccessKeyId", error.getCode());
}


TEST(OSSClientBucketVersioningTest, GetBucketVersioning_FullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<VersioningConfiguration>
  <Status>Enabled</Status>
</VersioningConfiguration>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketVersioningRequest();
    request.setBucket("bucket");
    auto outcome = client.getBucketVersioning(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_TRUE(result.hasVersioningConfiguration());
    EXPECT_EQ("Enabled", result.getVersioningConfiguration().status.value());
}


TEST(OSSClientBucketVersioningTest, GetBucketVersioning_EmptyXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<VersioningConfiguration>
</VersioningConfiguration>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketVersioningRequest();
    request.setBucket("bucket");
    auto outcome = client.getBucketVersioning(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_TRUE(result.hasVersioningConfiguration());
    EXPECT_FALSE(result.getVersioningConfiguration().status.has_value());
}


TEST(OSSClientBucketVersioningTest, GetBucketVersioning_ErrorXml) {
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

    auto request = models::GetBucketVersioningRequest();
    request.setBucket("bucket");
    auto outcome = client.getBucketVersioning(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ("XMLError:10", error.getCode());
    EXPECT_EQ("ERROR", error.getSnapshot());
    EXPECT_EQ("id-1234", error.getRequestId());
}


TEST(OSSClientBucketVersioningTest, GetBucketVersioning_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::GetBucketVersioningRequest();
    auto outcome = client.getBucketVersioning(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());
}


TEST(OSSClientBucketVersioningTest, ListObjectVersions_FullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListVersionsResult>
  <Name>test-bucket</Name>
  <Prefix></Prefix>
  <KeyMarker></KeyMarker>
  <VersionIdMarker></VersionIdMarker>
  <NextKeyMarker>next-key</NextKeyMarker>
  <NextVersionIdMarker>next-vid</NextVersionIdMarker>
  <MaxKeys>100</MaxKeys>
  <Delimiter>/</Delimiter>
  <IsTruncated>true</IsTruncated>
  <EncodingType>url</EncodingType>
  <Version>
    <Key>example%2Fobject1</Key>
    <VersionId>vid-001</VersionId>
    <IsLatest>true</IsLatest>
    <LastModified>2024-01-01T00:00:00.000Z</LastModified>
    <ETag>"etag-001"</ETag>
    <Size>1024</Size>
    <StorageClass>Standard</StorageClass>
    <Owner>
      <ID>owner-id</ID>
      <DisplayName>owner-name</DisplayName>
    </Owner>
  </Version>
  <Version>
    <Key>example%2Fobject2</Key>
    <VersionId>vid-002</VersionId>
    <IsLatest>false</IsLatest>
    <LastModified>2024-01-02T00:00:00.000Z</LastModified>
    <ETag>"etag-002"</ETag>
    <Size>2048</Size>
    <StorageClass>IA</StorageClass>
  </Version>
  <DeleteMarker>
    <Key>example%2Fdeleted</Key>
    <VersionId>vid-del-001</VersionId>
    <IsLatest>true</IsLatest>
    <LastModified>2024-01-03T00:00:00.000Z</LastModified>
    <Owner>
      <ID>owner-id</ID>
      <DisplayName>owner-name</DisplayName>
    </Owner>
  </DeleteMarker>
  <CommonPrefixes>
    <Prefix>example%2F</Prefix>
  </CommonPrefixes>
</ListVersionsResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    auto outcome = client.listObjectVersions(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("next-key", result.getNextKeyMarker());
    EXPECT_EQ("next-vid", result.getNextVersionIdMarker());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ("url", result.getEncodingType());

    // Versions (URL-decoded keys)
    ASSERT_EQ(2, result.getVersions().size());
    EXPECT_EQ("example/object1", result.getVersions()[0].key);
    EXPECT_EQ("vid-001", result.getVersions()[0].versionId);
    EXPECT_EQ(true, result.getVersions()[0].isLatest.value());
    EXPECT_EQ("2024-01-01T00:00:00.000Z", result.getVersions()[0].lastModified);
    EXPECT_EQ("\"etag-001\"", result.getVersions()[0].eTag);
    EXPECT_EQ(1024, result.getVersions()[0].size);
    EXPECT_EQ("Standard", result.getVersions()[0].storageClass);
    EXPECT_EQ("owner-id", result.getVersions()[0].owner.value().id);
    EXPECT_EQ("owner-name", result.getVersions()[0].owner.value().displayName);

    EXPECT_EQ("example/object2", result.getVersions()[1].key);
    EXPECT_EQ("vid-002", result.getVersions()[1].versionId);
    EXPECT_EQ(false, result.getVersions()[1].isLatest.value());
    EXPECT_EQ(2048, result.getVersions()[1].size);
    EXPECT_EQ("IA", result.getVersions()[1].storageClass);

    // Delete markers (URL-decoded keys)
    ASSERT_EQ(1, result.getDeleteMarkers().size());
    EXPECT_EQ("example/deleted", result.getDeleteMarkers()[0].key);
    EXPECT_EQ("vid-del-001", result.getDeleteMarkers()[0].versionId);
    EXPECT_EQ(true, result.getDeleteMarkers()[0].isLatest.value());
    EXPECT_EQ("owner-id", result.getDeleteMarkers()[0].owner.value().id);

    // Common prefixes (URL-decoded)
    ASSERT_EQ(1, result.getCommonPrefixes().size());
    EXPECT_EQ("example/", result.getCommonPrefixes()[0].prefix);
}


TEST(OSSClientBucketVersioningTest, ListObjectVersions_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::ListObjectVersionsRequest();
    auto outcome = client.listObjectVersions(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());
}


TEST(OSSClientBucketVersioningTest, ListObjectVersions_ErrorResponse) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<Error>
    <Code>NoSuchBucket</Code>
    <Message>The specified bucket does not exist.</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
    <BucketName>no-bucket</BucketName>
    <EC>0015-00000101</EC>
    <RecommendDoc>https://api.aliyun.com/troubleshoot?q=0015-00000101</RecommendDoc>
</Error>
)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(ResponseMessage{
            404, "Not Found", {{"x-oss-request-id", "id-12345"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("no-bucket");
    auto outcome = client.listObjectVersions(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(404, error.getStatusCode());
    EXPECT_EQ("ListObjectVersions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("NoSuchBucket", error.getCode());
}


} // namespace alibabacloud::oss2
