#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "MockAsyncTransport.h"

namespace alibabacloud::oss2 {

TEST(OSSAsyncClientBucketVersioningTest, PutBucketVersioningAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::PutBucketVersioningRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncClientBucketVersioningTest, PutBucketVersioningAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketVersioningRequest();
    request.setBucket("test-bucket");
    request.setVersioningConfiguration(models::VersioningConfiguration().setStatus("Enabled"));
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncClientBucketVersioningTest, GetBucketVersioningAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::GetBucketVersioningRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncClientBucketVersioningTest, GetBucketVersioningAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<VersioningConfiguration>
  <Status>Suspended</Status>
</VersioningConfiguration>)";

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketVersioningRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_TRUE(outcome.value().hasVersioningConfiguration());
    EXPECT_EQ("Suspended", outcome.value().getVersioningConfiguration().status.value());
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_RequiredField) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto request = models::ListObjectVersionsRequest();
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentRequired", outcome.error().getCode());
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_Success) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListVersionsResult>
  <Name>test-bucket</Name>
  <Prefix></Prefix>
  <KeyMarker></KeyMarker>
  <VersionIdMarker></VersionIdMarker>
  <MaxKeys>100</MaxKeys>
  <IsTruncated>false</IsTruncated>
  <EncodingType>url</EncodingType>
  <Version>
    <Key>test-key</Key>
    <VersionId>vid-001</VersionId>
    <IsLatest>true</IsLatest>
    <LastModified>2024-01-01T00:00:00.000Z</LastModified>
    <ETag>"etag-001"</ETag>
    <Size>512</Size>
    <StorageClass>Standard</StorageClass>
  </Version>
</ListVersionsResult>)";

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("test-bucket", outcome.value().getName());
    EXPECT_FALSE(outcome.value().getIsTruncated());
    ASSERT_EQ(1, outcome.value().getVersions().size());
    EXPECT_EQ("test-key", outcome.value().getVersions()[0].key);
    EXPECT_EQ("vid-001", outcome.value().getVersions()[0].versionId);
    EXPECT_EQ(512, outcome.value().getVersions()[0].size);
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_FullResponse) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListVersionsResult>
  <Name>test-bucket</Name>
  <Prefix>docs/</Prefix>
  <KeyMarker>key1</KeyMarker>
  <VersionIdMarker>vid-marker</VersionIdMarker>
  <NextKeyMarker>key2</NextKeyMarker>
  <NextVersionIdMarker>vid-next</NextVersionIdMarker>
  <MaxKeys>50</MaxKeys>
  <Delimiter>/</Delimiter>
  <IsTruncated>true</IsTruncated>
  <EncodingType>url</EncodingType>
  <Version>
    <Key>docs/file1.txt</Key>
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
    <RestoreInfo>ongoing-request="true"</RestoreInfo>
  </Version>
  <Version>
    <Key>docs/file2.txt</Key>
    <VersionId>vid-002</VersionId>
    <IsLatest>false</IsLatest>
    <LastModified>2024-01-02T00:00:00.000Z</LastModified>
    <ETag>"etag-002"</ETag>
    <Size>2048</Size>
    <StorageClass>IA</StorageClass>
  </Version>
  <DeleteMarker>
    <Key>docs/deleted.txt</Key>
    <VersionId>vid-del-001</VersionId>
    <IsLatest>true</IsLatest>
    <LastModified>2024-01-03T00:00:00.000Z</LastModified>
    <Owner>
      <ID>owner-id</ID>
      <DisplayName>owner-name</DisplayName>
    </Owner>
  </DeleteMarker>
  <CommonPrefixes>
    <Prefix>docs/subdir/</Prefix>
  </CommonPrefixes>
</ListVersionsResult>)";

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-full"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    request.addHeader("x-custom-header", "value");
    request.addParameter("custom-param", "pvalue");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ(2u, result.getVersions().size());
    EXPECT_FALSE(result.getVersions()[0].restoreInfo.empty());
    EXPECT_EQ(1u, result.getDeleteMarkers().size());
    EXPECT_EQ("docs/deleted.txt", result.getDeleteMarkers()[0].key);
    EXPECT_EQ(1u, result.getCommonPrefixes().size());
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_MinimalVersion) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListVersionsResult>
  <Name>test-bucket</Name>
  <IsTruncated>false</IsTruncated>
  <Version>
    <Key>file.txt</Key>
  </Version>
  <DeleteMarker>
    <Key>deleted.txt</Key>
  </DeleteMarker>
</ListVersionsResult>)";

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-min"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1u, outcome.value().getVersions().size());
    EXPECT_EQ(1u, outcome.value().getDeleteMarkers().size());
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_NullBody) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-null"}}, nullptr}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

TEST(OSSAsyncClientBucketVersioningTest, ListObjectVersionsAsync_InvalidXml) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-bad"}},
                           std::make_shared<std::stringstream>("<invalid xml")}));

    auto request = models::ListObjectVersionsRequest();
    request.setBucket("test-bucket");
    auto future = client.asyncCall(request);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
}

TEST(OSSAsyncClientBucketVersioningTest, PutBucketVersioningAsync_WithCustomHeaders) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-custom"}}, nullptr}));

    auto request = models::PutBucketVersioningRequest();
    request.setBucket("test-bucket");
    request.addHeader("x-custom", "val");
    request.addParameter("p1", "v1");
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSAsyncClientBucketVersioningTest, GetBucketVersioningAsync_NullBody) {
    auto mockTransport = std::make_shared<MockAsyncTransport>();
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mockTransport;
    auto client = OSSAsyncClient(config);

    mockTransport->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-null"}}, nullptr}));

    auto request = models::GetBucketVersioningRequest();
    request.setBucket("test-bucket");
    request.addHeader("x-h", "v");
    request.addParameter("p", "v");
    auto future = client.asyncCall(request);
    auto outcome = future.get();
    EXPECT_TRUE(outcome.has_value());
}

} // namespace alibabacloud::oss2
