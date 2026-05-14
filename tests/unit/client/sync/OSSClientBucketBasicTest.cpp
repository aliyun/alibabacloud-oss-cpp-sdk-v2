#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/models/BucketBasic.h"
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

TEST(OSSClientBucketBasicTest, GetBucketStat_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<BucketStat>
    <Storage>1024</Storage>
    <ObjectCount>10</ObjectCount>
    <MultipartUploadCount>5</MultipartUploadCount>
    <LiveChannelCount>2</LiveChannelCount>
    <LastModifiedTime>1678886400</LastModifiedTime>
    <StandardStorage>512</StandardStorage>
    <StandardObjectCount>3</StandardObjectCount>
    <InfrequentAccessStorage>256</InfrequentAccessStorage>
    <InfrequentAccessRealStorage>200</InfrequentAccessRealStorage>
    <InfrequentAccessObjectCount>2</InfrequentAccessObjectCount>
    <ArchiveStorage>128</ArchiveStorage>
    <ArchiveRealStorage>100</ArchiveRealStorage>
    <ArchiveObjectCount>2</ArchiveObjectCount>
    <ColdArchiveStorage>64</ColdArchiveStorage>
    <ColdArchiveRealStorage>50</ColdArchiveRealStorage>
    <ColdArchiveObjectCount>1</ColdArchiveObjectCount>
    <DeepColdArchiveStorage>32</DeepColdArchiveStorage>
    <DeepColdArchiveRealStorage>25</DeepColdArchiveRealStorage>
    <DeepColdArchiveObjectCount>1</DeepColdArchiveObjectCount>
    <DeleteMarkerCount>1</DeleteMarkerCount>
    <MultipartPartCount>3</MultipartPartCount>
    <MultipartPartStorage>16</MultipartPartStorage>
    <InfrequentMultipartPartCount>1</InfrequentMultipartPartCount>
    <InfrequentMultipartPartStorage>8</InfrequentMultipartPartStorage>
    <ArchiveMultipartPartCount>1</ArchiveMultipartPartCount>
    <ArchiveMultipartPartStorage>4</ArchiveMultipartPartStorage>
    <ColdArchiveMultipartPartCount>1</ColdArchiveMultipartPartCount>
    <ColdArchiveMultipartPartStorage>2</ColdArchiveMultipartPartStorage>
    <DeepColdArchiveMultipartPartCount>1</DeepColdArchiveMultipartPartCount>
    <DeepColdArchiveMultipartPartStorage>1</DeepColdArchiveMultipartPartStorage>
    <StandardMultipartPartCount>1</StandardMultipartPartCount>
    <StandardMultipartPartStorage>1</StandardMultipartPartStorage>
</BucketStat>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketStatRequest();
    request.setBucket("test-bucket");
    auto outcome = client.getBucketStat(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasBucketStat());
    EXPECT_EQ(1024, result.getBucketStat().storage.value());
    EXPECT_EQ(10, result.getBucketStat().objectCount.value());
    EXPECT_EQ(5, result.getBucketStat().multipartUploadCount.value());
    EXPECT_EQ(2, result.getBucketStat().liveChannelCount.value());
    EXPECT_EQ(1678886400, result.getBucketStat().lastModifiedTime.value());
    EXPECT_EQ(512, result.getBucketStat().standardStorage.value());
    EXPECT_EQ(3, result.getBucketStat().standardObjectCount.value());
    EXPECT_EQ(256, result.getBucketStat().infrequentAccessStorage.value());
    EXPECT_EQ(200, result.getBucketStat().infrequentAccessRealStorage.value());
    EXPECT_EQ(2, result.getBucketStat().infrequentAccessObjectCount.value());
    EXPECT_EQ(128, result.getBucketStat().archiveStorage.value());
    EXPECT_EQ(100, result.getBucketStat().archiveRealStorage.value());
    EXPECT_EQ(2, result.getBucketStat().archiveObjectCount.value());
    EXPECT_EQ(64, result.getBucketStat().coldArchiveStorage.value());
    EXPECT_EQ(50, result.getBucketStat().coldArchiveRealStorage.value());
    EXPECT_EQ(1, result.getBucketStat().coldArchiveObjectCount.value());
    EXPECT_EQ(32, result.getBucketStat().deepColdArchiveStorage.value());
    EXPECT_EQ(25, result.getBucketStat().deepColdArchiveRealStorage.value());
    EXPECT_EQ(1, result.getBucketStat().deepColdArchiveObjectCount.value());
    EXPECT_EQ(1, result.getBucketStat().deleteMarkerCount.value());
    EXPECT_EQ(3, result.getBucketStat().multipartPartCount.value());
    EXPECT_EQ(16, result.getBucketStat().multipartPartStorage.value());
    EXPECT_EQ(1, result.getBucketStat().infrequentMultipartPartCount.value());
    EXPECT_EQ(8, result.getBucketStat().infrequentMultipartPartStorage.value());
    EXPECT_EQ(1, result.getBucketStat().archiveMultipartPartCount.value());
    EXPECT_EQ(4, result.getBucketStat().archiveMultipartPartStorage.value());
    EXPECT_EQ(1, result.getBucketStat().coldArchiveMultipartPartCount.value());
    EXPECT_EQ(2, result.getBucketStat().coldArchiveMultipartPartStorage.value());
    EXPECT_EQ(1, result.getBucketStat().deepColdArchiveMultipartPartCount.value());
    EXPECT_EQ(1, result.getBucketStat().deepColdArchiveMultipartPartStorage.value());
    EXPECT_EQ(1, result.getBucketStat().standardMultipartPartCount.value());
    EXPECT_EQ(1, result.getBucketStat().standardMultipartPartStorage.value());
}

TEST(OSSClientBucketBasicTest, GetBucketStat_ErrorResponse) {
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

    auto request = models::GetBucketStatRequest();
    request.setBucket("test-bucket");
    auto outcome = client.getBucketStat(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("GetBucketStat", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/?stat", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, GetBucketStat_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<BucketStat>
    <Storage>1024</Storage>
</BucketStat>
    )";
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketStatRequest();
    auto outcome = client.getBucketStat(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.getBucketStat(request);
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientBucketBasicTest, PutBucket_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketRequest();
    request.setBucket("test-bucket").setAcl("private");

    auto outcome = client.putBucket(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientBucketBasicTest, PutBucket_WithConfiguration) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::PutBucketRequest();
    request.setBucket("test-bucket").setAcl("public-read").setResourceGroupId("rg-123").setBucketTagging("k1=v1&k2=v2");

    // Add configuration
    models::CreateBucketConfiguration configObj;
    configObj.setStorageClass("IA").setDataRedundancyType("ZRS");
    request.setCreateBucketConfiguration(configObj);

    auto outcome = client.putBucket(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientBucketBasicTest, PutBucket_ErrorResponse) {
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

    auto request = models::PutBucketRequest();
    request.setBucket("test-bucket");
    auto outcome = client.putBucket(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("PutBucket", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, PutBucket_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>("")}));

    auto request = models::PutBucketRequest();
    auto outcome = client.putBucket(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.putBucket(request);
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientBucketBasicTest, DeleteBucket_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::DeleteBucketRequest();
    request.setBucket("test-bucket");

    auto outcome = client.deleteBucket(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
}

TEST(OSSClientBucketBasicTest, DeleteBucket_ErrorResponse) {
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

    auto request = models::DeleteBucketRequest();
    request.setBucket("test-bucket");
    auto outcome = client.deleteBucket(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("DeleteBucket", error.getOpName());
    EXPECT_EQ("DELETE", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, DeleteBucket_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::DeleteBucketRequest();
    auto outcome = client.deleteBucket(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.deleteBucket(request);
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientBucketBasicTest, ListObjects_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <Prefix>prefix</Prefix>
    <Marker>prefix/</Marker>
    <MaxKeys>100</MaxKeys>
    <Delimiter>/</Delimiter>
    <IsTruncated>true</IsTruncated>
    <NextMarker>prefix/folder1/object2.txt</NextMarker>
    <Contents>
        <Key>prefix/object1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
        <Owner>
            <ID>owner-id</ID>
            <DisplayName>owner-display-name</DisplayName>
        </Owner>
    </Contents>
    <Contents>
        <Key>prefix/folder1/object2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1/</Prefix>
    </CommonPrefixes>
    <CommonPrefixes>
        <Prefix>folder2/</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsRequest();
    request.setBucket("test-bucket").setDelimiter("/").setMaxKeys(100);

    auto outcome = client.listObjects(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("prefix", result.getPrefix());
    EXPECT_EQ("prefix/", result.getMarker());
    EXPECT_EQ("prefix/folder1/object2.txt", result.getNextMarker());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(2, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("prefix/object1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);
    EXPECT_EQ("owner-id", result.getContents()[0].owner.value().id);
    EXPECT_EQ("owner-display-name", result.getContents()[0].owner.value().displayName);

    // Check second object
    EXPECT_EQ("prefix/folder1/object2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1/", result.getCommonPrefixes()[0].prefix);
    EXPECT_EQ("folder2/", result.getCommonPrefixes()[1].prefix);
}

TEST(OSSClientBucketBasicTest, ListObjects_Success_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <Prefix>prefix%2F</Prefix>
    <Marker>prefix%2F</Marker>
    <MaxKeys>100</MaxKeys>
    <Delimiter>%2F</Delimiter>
    <IsTruncated>true</IsTruncated>
    <EncodingType>url</EncodingType>
    <NextMarker>prefix%2Ffolder1%2Fobject2.txt</NextMarker>
    <Contents>
        <Key>prefix%2Fobject1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
        <Owner>
            <ID>owner-id</ID>
            <DisplayName>owner-display-name</DisplayName>
        </Owner>
    </Contents>
    <Contents>
        <Key>prefix%2Ffolder1%2Fobject2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1%2F</Prefix>
    </CommonPrefixes>
    <CommonPrefixes>
        <Prefix>folder2%2F</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsRequest();
    request.setBucket("test-bucket").setDelimiter("/").setMaxKeys(100);

    auto outcome = client.listObjects(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("prefix/", result.getPrefix());
    EXPECT_EQ("prefix/", result.getMarker());
    EXPECT_EQ("prefix/folder1/object2.txt", result.getNextMarker());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(2, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("prefix/object1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);
    EXPECT_EQ("owner-id", result.getContents()[0].owner.value().id);
    EXPECT_EQ("owner-display-name", result.getContents()[0].owner.value().displayName);

    // Check second object
    EXPECT_EQ("prefix/folder1/object2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1/", result.getCommonPrefixes()[0].prefix);
    EXPECT_EQ("folder2/", result.getCommonPrefixes()[1].prefix);
}

TEST(OSSClientBucketBasicTest, ListObjects_Success_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <Prefix>prefix%2F</Prefix>
    <Marker>prefix%2F</Marker>
    <MaxKeys>100</MaxKeys>
    <Delimiter>%2F</Delimiter>
    <IsTruncated>true</IsTruncated>
    <EncodingType></EncodingType>
    <NextMarker>prefix%2Ffolder1%2Fobject2.txt</NextMarker>
    <Contents>
        <Key>prefix%2Fobject1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
        <Owner>
            <ID>owner-id</ID>
            <DisplayName>owner-display-name</DisplayName>
        </Owner>
    </Contents>
    <Contents>
        <Key>prefix%2Ffolder1%2Fobject2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1%2F</Prefix>
    </CommonPrefixes>
    <CommonPrefixes>
        <Prefix>folder2%2F</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsRequest();
    request.setBucket("test-bucket").setDelimiter("/").setMaxKeys(100);

    auto outcome = client.listObjects(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("prefix%2F", result.getPrefix());
    EXPECT_EQ("prefix%2F", result.getMarker());
    EXPECT_EQ("prefix%2Ffolder1%2Fobject2.txt", result.getNextMarker());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("%2F", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(2, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("prefix%2Fobject1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);
    EXPECT_EQ("owner-id", result.getContents()[0].owner.value().id);
    EXPECT_EQ("owner-display-name", result.getContents()[0].owner.value().displayName);

    // Check second object
    EXPECT_EQ("prefix%2Ffolder1%2Fobject2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1%2F", result.getCommonPrefixes()[0].prefix);
    EXPECT_EQ("folder2%2F", result.getCommonPrefixes()[1].prefix);
}


TEST(OSSClientBucketBasicTest, ListObjects_ErrorResponse) {
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

    auto request = models::ListObjectsRequest();
    request.setBucket("test-bucket");
    auto outcome = client.listObjects(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListObjects", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/?encoding-type=url", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, ListObjects_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::ListObjectsRequest();
    auto outcome = client.listObjects(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.listObjects(request);
    EXPECT_TRUE(outcome.isSuccess());
}

TEST(OSSClientBucketBasicTest, ListObjectsV2_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket-v2</Name>
    <Prefix></Prefix>
    <KeyCount>2</KeyCount>
    <MaxKeys>100</MaxKeys>
    <Delimiter>/</Delimiter>
    <IsTruncated>false</IsTruncated>
    <Contents>
        <Key>v2-object1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"v2-etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
    </Contents>
    <Contents>
        <Key>folder1/v2-object2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"v2-etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1/</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsV2Request();
    request.setBucket("test-bucket-v2").setDelimiter("/").setMaxKeys(100).setFetchOwner(false);

    auto outcome = client.listObjectsV2(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket-v2", result.getName());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_EQ(2, result.getKeyCount());
    EXPECT_FALSE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(1, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("v2-object1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"v2-etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);

    // Check second object
    EXPECT_EQ("folder1/v2-object2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"v2-etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1/", result.getCommonPrefixes()[0].prefix);
}

TEST(OSSClientBucketBasicTest, ListObjectsV2_Success_EncodingTypeUrl) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket-v2</Name>
    <Prefix>123%2F</Prefix>
    <KeyCount>2</KeyCount>
    <MaxKeys>100</MaxKeys>
    <Delimiter>%2F</Delimiter>
    <IsTruncated>false</IsTruncated>
    <StartAfter>123%2F123</StartAfter>
    <ContinuationToken>123%2F1234</ContinuationToken>
    <NextContinuationToken>123%2F12345</NextContinuationToken>
    <EncodingType>url</EncodingType>
    <Contents>
        <Key>123%2Fv2-object1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"v2-etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
    </Contents>
    <Contents>
        <Key>folder1%2Fv2-object2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"v2-etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1%2F</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsV2Request();
    request.setBucket("test-bucket-v2").setDelimiter("/").setMaxKeys(100).setFetchOwner(false);

    auto outcome = client.listObjectsV2(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket-v2", result.getName());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("123/", result.getPrefix());
    EXPECT_EQ("123/123", result.getStartAfter());
    EXPECT_EQ("123/1234", result.getContinuationToken());
    EXPECT_EQ("123/12345", result.getNextContinuationToken());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_EQ(2, result.getKeyCount());
    EXPECT_FALSE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(1, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("123/v2-object1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"v2-etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);

    // Check second object
    EXPECT_EQ("folder1/v2-object2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"v2-etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1/", result.getCommonPrefixes()[0].prefix);
}


TEST(OSSClientBucketBasicTest, ListObjectsV2_Success_EncodingTypeNone) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket-v2</Name>
    <Prefix>123%2F</Prefix>
    <KeyCount>2</KeyCount>
    <MaxKeys>100</MaxKeys>
    <Delimiter>%2F</Delimiter>
    <IsTruncated>false</IsTruncated>
    <StartAfter>123%2F123</StartAfter>
    <ContinuationToken>123%2F1234</ContinuationToken>
    <NextContinuationToken>123%2F12345</NextContinuationToken>
    <Contents>
        <Key>123%2Fv2-object1.txt</Key>
        <LastModified>2023-01-01T00:00:00.000Z</LastModified>
        <ETag>"v2-etag1"</ETag>
        <Type>Normal</Type>
        <Size>1024</Size>
        <StorageClass>Standard</StorageClass>
    </Contents>
    <Contents>
        <Key>folder1%2Fv2-object2.txt</Key>
        <LastModified>2023-01-02T00:00:00.000Z</LastModified>
        <ETag>"v2-etag2"</ETag>
        <Type>Normal</Type>
        <Size>2048</Size>
        <StorageClass>IA</StorageClass>
    </Contents>
    <CommonPrefixes>
        <Prefix>folder1%2F</Prefix>
    </CommonPrefixes>
</ListBucketResult>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsV2Request();
    request.setBucket("test-bucket-v2").setDelimiter("/").setMaxKeys(100).setFetchOwner(false);

    auto outcome = client.listObjectsV2(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("test-bucket-v2", result.getName());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("123%2F", result.getPrefix());
    EXPECT_EQ("123%2F123", result.getStartAfter());
    EXPECT_EQ("123%2F1234", result.getContinuationToken());
    EXPECT_EQ("123%2F12345", result.getNextContinuationToken());
    EXPECT_EQ("%2F", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_EQ(2, result.getKeyCount());
    EXPECT_FALSE(result.getIsTruncated());
    EXPECT_EQ(2, result.getContents().size());
    EXPECT_EQ(1, result.getCommonPrefixes().size());

    // Check first object
    EXPECT_EQ("123%2Fv2-object1.txt", result.getContents()[0].key);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getContents()[0].lastModified);
    EXPECT_EQ("\"v2-etag1\"", result.getContents()[0].eTag);
    EXPECT_EQ(1024, result.getContents()[0].size);
    EXPECT_EQ("Standard", result.getContents()[0].storageClass);

    // Check second object
    EXPECT_EQ("folder1%2Fv2-object2.txt", result.getContents()[1].key);
    EXPECT_EQ("2023-01-02T00:00:00.000Z", result.getContents()[1].lastModified);
    EXPECT_EQ("\"v2-etag2\"", result.getContents()[1].eTag);
    EXPECT_EQ(2048, result.getContents()[1].size);
    EXPECT_EQ("IA", result.getContents()[1].storageClass);

    // Check common prefixes
    EXPECT_EQ("folder1%2F", result.getCommonPrefixes()[0].prefix);
}

TEST(OSSClientBucketBasicTest, ListObjectsV2_ErrorResponse) {
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

    auto request = models::ListObjectsV2Request();
    request.setBucket("test-bucket");
    auto outcome = client.listObjectsV2(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("ListObjectsV2", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://test-bucket.oss-cn-hangzhou.aliyuncs.com/?encoding-type=url&list-type=2", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, ListObjectsV2_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::ListObjectsV2Request();
    auto outcome = client.listObjectsV2(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.listObjectsV2(request);
    EXPECT_TRUE(outcome.isSuccess());
}


TEST(OSSClientBucketBasicTest, GetBucketInfo_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<BucketInfo>
    <Bucket>
        <CreationDate>2023-01-01T00:00:00.000Z</CreationDate>
        <ExtranetEndpoint>test-bucket.oss-cn-hangzhou.aliyuncs.com</ExtranetEndpoint>
        <IntranetEndpoint>test-bucket.oss-cn-hangzhou-internal.aliyuncs.com</IntranetEndpoint>
        <Location>oss-cn-hangzhou</Location>
        <Name>test-bucket</Name>
        <StorageClass>Standard</StorageClass>
        <DataRedundancyType>LRS</DataRedundancyType>
        <Comment>Test bucket comment</Comment>
        <Owner>
            <DisplayName>test-user</DisplayName>
            <ID>test-user-id</ID>
        </Owner>
        <TransferAcceleration>Disabled</TransferAcceleration>
        <AccessMonitor>Disabled</AccessMonitor>
        <ResourceGroupId>rg-123</ResourceGroupId>
        <CrossRegionReplication>Disabled</CrossRegionReplication>
        <Versioning>Enabled</Versioning>
        <BlockPublicAccess>true</BlockPublicAccess>
    </Bucket>
</BucketInfo>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketInfoRequest();
    request.setBucket("test-bucket");

    auto outcome = client.getBucketInfo(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasBucketInfo());
    EXPECT_EQ("test-bucket", result.getBucketInfo().name);
    EXPECT_EQ("oss-cn-hangzhou", result.getBucketInfo().location);
    EXPECT_EQ("Standard", result.getBucketInfo().storageClass);
    EXPECT_EQ("LRS", result.getBucketInfo().dataRedundancyType);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", result.getBucketInfo().creationDate);
    EXPECT_EQ("test-bucket.oss-cn-hangzhou.aliyuncs.com", result.getBucketInfo().extranetEndpoint);
    EXPECT_EQ("test-bucket.oss-cn-hangzhou-internal.aliyuncs.com", result.getBucketInfo().intranetEndpoint);
    EXPECT_EQ("Test bucket comment", result.getBucketInfo().comment);
    EXPECT_EQ("test-user", result.getBucketInfo().owner.displayName);
    EXPECT_EQ("test-user-id", result.getBucketInfo().owner.id);
    EXPECT_EQ("Disabled", result.getBucketInfo().transferAcceleration.value());
    EXPECT_EQ("Disabled", result.getBucketInfo().accessMonitor.value());
    EXPECT_EQ("rg-123", result.getBucketInfo().resourceGroupId.value());
    EXPECT_EQ("Disabled", result.getBucketInfo().crossRegionReplication.value());
    EXPECT_EQ("Enabled", result.getBucketInfo().versioning.value());
    EXPECT_EQ(true, result.getBucketInfo().blockPublicAccess.value());
}

TEST(OSSClientBucketBasicTest, GetBucketInfo_ErrorResponse) {
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

    auto request = models::GetBucketInfoRequest();
    request.setBucket("bucket");
    auto outcome = client.getBucketInfo(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("GetBucketInfo", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?bucketInfo", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, GetBucketInfo_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{204, "No Content", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto request = models::GetBucketInfoRequest();
    auto outcome = client.getBucketInfo(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.getBucketInfo(request);
    EXPECT_TRUE(outcome.isSuccess());
}


TEST(OSSClientBucketBasicTest, GetBucketLocation_Success) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<LocationConstraint>oss-cn-hangzhou</LocationConstraint>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketLocationRequest();
    request.setBucket("test-bucket");

    auto outcome = client.getBucketLocation(request);
    EXPECT_TRUE(outcome.isSuccess());
    auto& result = outcome.getResult();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ("oss-cn-hangzhou", result.getLocationConstraint());
}


TEST(OSSClientBucketBasicTest, GetBucketLocation_ErrorResponse) {
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

    auto request = models::GetBucketLocationRequest();
    request.setBucket("bucket");
    auto outcome = client.getBucketLocation(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("GetBucketLocation", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://bucket.oss-cn-hangzhou.aliyuncs.com/?location", error.getRequestTarget());
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

TEST(OSSClientBucketBasicTest, GetBucketLocation_RequiredField) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);
    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<LocationConstraint>oss-cn-hangzhou</LocationConstraint>
    )";
    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::GetBucketLocationRequest();
    auto outcome = client.getBucketLocation(request);
    EXPECT_FALSE(outcome.isSuccess());
    auto& error = outcome.getError();

    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("ArgumentRequired", error.getCode());
    EXPECT_EQ("Missing field Bucket", error.getMessage());

    request.setBucket("test-bucket");
    outcome = client.getBucketLocation(request);
    EXPECT_TRUE(outcome.isSuccess());
}

} // namespace alibabacloud::oss2