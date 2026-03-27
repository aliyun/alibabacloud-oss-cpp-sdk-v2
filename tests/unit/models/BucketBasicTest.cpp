#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/BucketBasic.h"


namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(BucketBasicTest, ServerSideEncryptionRule_ConstructorDefault) {
    auto rule = ServerSideEncryptionRule();
    EXPECT_EQ(false, rule.sseAlgorithm.has_value());
    EXPECT_EQ(false, rule.kmsMasterKeyID.has_value());
    EXPECT_EQ(false, rule.kmsDataEncryption.has_value());
}

TEST(BucketBasicTest, ServerSideEncryptionRule_Setter) {
    auto rule = ServerSideEncryptionRule();
    EXPECT_EQ(false, rule.sseAlgorithm.has_value());
    EXPECT_EQ(false, rule.kmsMasterKeyID.has_value());
    EXPECT_EQ(false, rule.kmsDataEncryption.has_value());

    // Setter
    rule.setSSEAlgorithm("AES-256").setKMSMasterKeyID("key-123").setKMSDataEncryption("SM4");

    EXPECT_EQ("AES-256", rule.sseAlgorithm.value());
    EXPECT_EQ("key-123", rule.kmsMasterKeyID.value());
    EXPECT_EQ("SM4", rule.kmsDataEncryption.value());
}

TEST(BucketBasicTest, BucketStat_ConstructorDefault) {
    auto stat = BucketStat();
    EXPECT_EQ(false, stat.objectCount.has_value());
    EXPECT_EQ(false, stat.archiveStorage.has_value());
    EXPECT_EQ(false, stat.coldArchiveObjectCount.has_value());
    EXPECT_EQ(false, stat.deleteMarkerCount.has_value());
    EXPECT_EQ(false, stat.standardStorage.has_value());
    EXPECT_EQ(false, stat.infrequentAccessRealStorage.has_value());
    EXPECT_EQ(false, stat.multipartPartStorage.has_value());
    EXPECT_EQ(false, stat.infrequentMultipartPartStorage.has_value());
    EXPECT_EQ(false, stat.archiveMultipartPartCount.has_value());
    EXPECT_EQ(false, stat.deepColdArchiveMultipartPartCount.has_value());
    EXPECT_EQ(false, stat.deepColdArchiveRealStorage.has_value());
    EXPECT_EQ(false, stat.infrequentMultipartPartCount.has_value());
    EXPECT_EQ(false, stat.infrequentAccessStorage.has_value());
    EXPECT_EQ(false, stat.archiveObjectCount.has_value());
    EXPECT_EQ(false, stat.coldArchiveRealStorage.has_value());
    EXPECT_EQ(false, stat.deepColdArchiveStorage.has_value());
    EXPECT_EQ(false, stat.deepColdArchiveObjectCount.has_value());
    EXPECT_EQ(false, stat.storage.has_value());
    EXPECT_EQ(false, stat.liveChannelCount.has_value());
    EXPECT_EQ(false, stat.lastModifiedTime.has_value());
    EXPECT_EQ(false, stat.deepColdArchiveMultipartPartStorage.has_value());
    EXPECT_EQ(false, stat.standardObjectCount.has_value());
    EXPECT_EQ(false, stat.archiveRealStorage.has_value());
    EXPECT_EQ(false, stat.archiveMultipartPartStorage.has_value());
    EXPECT_EQ(false, stat.multipartUploadCount.has_value());
    EXPECT_EQ(false, stat.multipartPartCount.has_value());
    EXPECT_EQ(false, stat.infrequentAccessObjectCount.has_value());
    EXPECT_EQ(false, stat.coldArchiveMultipartPartCount.has_value());
    EXPECT_EQ(false, stat.coldArchiveMultipartPartStorage.has_value());
    EXPECT_EQ(false, stat.coldArchiveStorage.has_value());
    EXPECT_EQ(false, stat.standardMultipartPartCount.has_value());
    EXPECT_EQ(false, stat.standardMultipartPartStorage.has_value());
}

TEST(BucketBasicTest, BucketStat_Setter) {
    auto stat = BucketStat();
    EXPECT_EQ(false, stat.objectCount.has_value());
    EXPECT_EQ(false, stat.storage.has_value());
    EXPECT_EQ(false, stat.lastModifiedTime.has_value());

    // Setter
    stat.setObjectCount(100).setStorage(1024).setLastModifiedTime(1678886400);

    EXPECT_EQ(100, stat.objectCount.value());
    EXPECT_EQ(1024, stat.storage.value());
    EXPECT_EQ(1678886400, stat.lastModifiedTime.value());
}

TEST(BucketBasicTest, BucketPolicy_ConstructorDefault) {
    auto policy = BucketPolicy();
    EXPECT_EQ(false, policy.logPrefix.has_value());
    EXPECT_EQ(false, policy.logBucket.has_value());
}

TEST(BucketBasicTest, BucketPolicy_Setter) {
    auto policy = BucketPolicy();
    EXPECT_EQ(false, policy.logPrefix.has_value());
    EXPECT_EQ(false, policy.logBucket.has_value());

    // Setter
    policy.setLogPrefix("access-log-").setLogBucket("log-bucket");

    EXPECT_EQ("access-log-", policy.logPrefix.value());
    EXPECT_EQ("log-bucket", policy.logBucket.value());
}

TEST(BucketBasicTest, CreateBucketConfiguration_ConstructorDefault) {
    auto config = CreateBucketConfiguration();
    EXPECT_EQ(false, config.storageClass.has_value());
    EXPECT_EQ(false, config.dataRedundancyType.has_value());
}

TEST(BucketBasicTest, CreateBucketConfiguration_Setter) {
    auto config = CreateBucketConfiguration();
    EXPECT_EQ(false, config.storageClass.has_value());
    EXPECT_EQ(false, config.dataRedundancyType.has_value());

    // Setter
    config.setStorageClass("IA").setDataRedundancyType("ZRS");

    EXPECT_EQ("IA", config.storageClass.value());
    EXPECT_EQ("ZRS", config.dataRedundancyType.value());
}

TEST(BucketBasicTest, ObjectSummary_ConstructorDefault) {
    auto summary = ObjectSummary();
    EXPECT_EQ("", summary.key);
    EXPECT_EQ("", summary.type);
    EXPECT_EQ(0, summary.size);
    EXPECT_EQ("", summary.lastModified);
    EXPECT_EQ("", summary.eTag);
    EXPECT_EQ("", summary.storageClass);
    EXPECT_EQ(false, summary.owner.has_value());
    EXPECT_EQ(false, summary.restoreInfo.has_value());
    EXPECT_EQ(false, summary.transitionTime.has_value());
}

TEST(BucketBasicTest, ObjectSummary_Setter) {
    auto summary = ObjectSummary();
    EXPECT_EQ("", summary.key);
    EXPECT_EQ("", summary.type);
    EXPECT_EQ(0, summary.size);
    EXPECT_EQ("", summary.lastModified);
    EXPECT_EQ("", summary.eTag);
    EXPECT_EQ("", summary.storageClass);

    // Setter
    summary.setKey("test-key")
            .setType("Normal")
            .setSize(1024)
            .setLastModified("2023-01-01T00:00:00.000Z")
            .setETag("etag-123")
            .setStorageClass("Standard");

    EXPECT_EQ("test-key", summary.key);
    EXPECT_EQ("Normal", summary.type);
    EXPECT_EQ(1024, summary.size);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", summary.lastModified);
    EXPECT_EQ("etag-123", summary.eTag);
    EXPECT_EQ("Standard", summary.storageClass);
}

TEST(BucketBasicTest, ObjectSummary_Setter_WithOwner) {
    auto summary = ObjectSummary();
    EXPECT_EQ(false, summary.owner.has_value());

    // Setter with optional fields
    Owner owner;
    owner.id = "owner-id";
    owner.displayName = "owner-name";
    summary.setOwner(owner);

    EXPECT_EQ(true, summary.owner.has_value());
    EXPECT_EQ("owner-id", summary.owner.value().id);
    EXPECT_EQ("owner-name", summary.owner.value().displayName);
}

TEST(BucketBasicTest, BucketInfo_ConstructorDefault) {
    auto info = BucketInfo();
    EXPECT_EQ("", info.location);
    EXPECT_EQ("", info.name);
    EXPECT_EQ("", info.storageClass);
    EXPECT_EQ("", info.dataRedundancyType);
    EXPECT_EQ("", info.creationDate);
    EXPECT_EQ("", info.extranetEndpoint);
    EXPECT_EQ("", info.intranetEndpoint);
    EXPECT_EQ("", info.comment);
    EXPECT_EQ("", info.owner.id.value_or(""));
    EXPECT_EQ("", info.owner.displayName.value_or(""));
    EXPECT_EQ(false, info.transferAcceleration.has_value());
    EXPECT_EQ(false, info.accessMonitor.has_value());
    EXPECT_EQ(false, info.resourceGroupId.has_value());
    EXPECT_EQ(false, info.accessControlList.has_value());
    EXPECT_EQ(false, info.blockPublicAccess.has_value());
    EXPECT_EQ(false, info.crossRegionReplication.has_value());
    EXPECT_EQ(false, info.serverSideEncryptionRule.has_value());
    EXPECT_EQ(false, info.bucketPolicy.has_value());
    EXPECT_EQ(false, info.versioning.has_value());
}

TEST(BucketBasicTest, BucketInfo_Setter) {
    auto info = BucketInfo();
    EXPECT_EQ("", info.location);
    EXPECT_EQ("", info.name);
    EXPECT_EQ("", info.storageClass);

    // Set basic fields
    info.setLocation("oss-cn-hangzhou")
            .setName("test-bucket")
            .setStorageClass("Standard")
            .setDataRedundancyType("LRS")
            .setCreationDate("2023-01-01T00:00:00.000Z")
            .setExtranetEndpoint("test-bucket.oss-cn-hangzhou.aliyuncs.com")
            .setIntranetEndpoint("test-bucket.oss-cn-hangzhou-internal.aliyuncs.com")
            .setComment("test comment");

    EXPECT_EQ("oss-cn-hangzhou", info.location);
    EXPECT_EQ("test-bucket", info.name);
    EXPECT_EQ("Standard", info.storageClass);
    EXPECT_EQ("LRS", info.dataRedundancyType);
    EXPECT_EQ("2023-01-01T00:00:00.000Z", info.creationDate);
    EXPECT_EQ("test-bucket.oss-cn-hangzhou.aliyuncs.com", info.extranetEndpoint);
    EXPECT_EQ("test-bucket.oss-cn-hangzhou-internal.aliyuncs.com", info.intranetEndpoint);
    EXPECT_EQ("test comment", info.comment);
}

TEST(BucketBasicTest, BucketInfo_Setter_WithOptionalFields) {
    auto info = BucketInfo();
    EXPECT_EQ(false, info.transferAcceleration.has_value());
    EXPECT_EQ(false, info.accessControlList.has_value());

    // Set optional fields
    info.setTransferAcceleration("Enabled")
            .setAccessMonitor("Enabled")
            .setResourceGroupId("rg-123")
            .setBlockPublicAccess(true)
            .setCrossRegionReplication("Disabled")
            .setVersioning("Enabled");

    EXPECT_EQ("Enabled", info.transferAcceleration.value());
    EXPECT_EQ("Enabled", info.accessMonitor.value());
    EXPECT_EQ("rg-123", info.resourceGroupId.value());
    EXPECT_EQ(true, info.blockPublicAccess.value());
    EXPECT_EQ("Disabled", info.crossRegionReplication.value());
    EXPECT_EQ("Enabled", info.versioning.value());
}

TEST(BucketBasicTest, GetBucketStatRequest_ConstructorDefault) {
    // Default
    auto request = GetBucketStatRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, GetBucketStatRequest_Setter) {
    auto request = GetBucketStatRequest();
    EXPECT_EQ("", request.getBucket());

    // Setter
    request.setBucket("test-bucket");
    EXPECT_EQ("test-bucket", request.getBucket());
}

TEST(BucketBasicTest, GetBucketStatResult_ConstructorDefault) {
    auto result = GetBucketStatResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasBucketStat());
}

TEST(BucketBasicTest, GetBucketStatResult_ConstructorAll) {
    auto result = GetBucketStatResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasBucketStat());
}

TEST(BucketBasicTest, GetBucketStatResult_SetBody) {
    auto result = GetBucketStatResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    BucketStat stat;
    stat.setObjectCount(100).setStorage(1024);
    result.setBucketStat(stat);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasBucketStat());
    EXPECT_EQ(100, result.getBucketStat().objectCount.value());
    EXPECT_EQ(1024, result.getBucketStat().storage.value());
}

TEST(BucketBasicTest, PutBucketRequest_ConstructorDefault) {
    // Default
    auto request = PutBucketRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getAcl());
    EXPECT_EQ("", request.getResourceGroupId());
    EXPECT_EQ("", request.getBucketTagging());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
    EXPECT_EQ(false, request.hasCreateBucketConfiguration());
}

TEST(BucketBasicTest, PutBucketRequest_Setter) {
    auto request = PutBucketRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getAcl());

    // Setter
    request.setBucket("test-bucket").setAcl("public-read").setResourceGroupId("rg-123").setBucketTagging("k1=v1&k2=v2");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("public-read", request.getAcl());
    EXPECT_EQ("rg-123", request.getResourceGroupId());
    EXPECT_EQ("k1=v1&k2=v2", request.getBucketTagging());
}

TEST(BucketBasicTest, PutBucketRequest_SetCreateBucketConfiguration) {
    auto request = PutBucketRequest();
    EXPECT_EQ(false, request.hasCreateBucketConfiguration());

    CreateBucketConfiguration config;
    config.setStorageClass("IA").setDataRedundancyType("ZRS");
    request.setCreateBucketConfiguration(config);

    EXPECT_EQ(true, request.hasCreateBucketConfiguration());
    EXPECT_EQ("IA", request.getCreateBucketConfiguration().storageClass.value());
    EXPECT_EQ("ZRS", request.getCreateBucketConfiguration().dataRedundancyType.value());
}

TEST(BucketBasicTest, PutBucketResult_ConstructorDefault) {
    auto result = PutBucketResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(BucketBasicTest, PutBucketResult_ConstructorAll) {
    auto result = PutBucketResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}

TEST(BucketBasicTest, DeleteBucketRequest_ConstructorDefault) {
    // Default
    auto request = DeleteBucketRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, DeleteBucketRequest_Setter) {
    auto request = DeleteBucketRequest();
    EXPECT_EQ("", request.getBucket());

    // Setter
    request.setBucket("test-bucket");
    EXPECT_EQ("test-bucket", request.getBucket());
}

TEST(BucketBasicTest, DeleteBucketResult_ConstructorDefault) {
    auto result = DeleteBucketResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(BucketBasicTest, DeleteBucketResult_ConstructorAll) {
    auto result = DeleteBucketResult(204, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}

TEST(BucketBasicTest, ListObjectsRequest_ConstructorDefault) {
    // Default
    auto request = ListObjectsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ("", request.getMarker());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(-1, request.getMaxKeys());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, ListObjectsRequest_Setter) {
    auto request = ListObjectsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ(-1, request.getMaxKeys());

    // Setter
    request.setBucket("test-bucket")
            .setDelimiter("/")
            .setMarker("marker-123")
            .setMaxKeys(100)
            .setPrefix("prefix-")
            .setEncodingType("url");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("/", request.getDelimiter());
    EXPECT_EQ("marker-123", request.getMarker());
    EXPECT_EQ(100, request.getMaxKeys());
    EXPECT_EQ("prefix-", request.getPrefix());
    EXPECT_EQ("url", request.getEncodingType());
}


TEST(BucketBasicTest, ListObjectsResult_ConstructorDefault) {
    auto result = ListObjectsResult();
    EXPECT_EQ("", result.getName());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("", result.getNextMarker());
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getMarker());
    EXPECT_EQ("", result.getDelimiter());
    EXPECT_EQ(-1, result.getMaxKeys());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(0, result.getContents().size());
    EXPECT_EQ(0, result.getCommonPrefixes().size());
}

TEST(BucketBasicTest, ListObjectsResult_ConstructorAll) {
    CommonPrefix prefix;
    prefix.prefix = "test-prefix/";
    std::vector<CommonPrefix> prefixes = {prefix};

    ObjectSummary summary;
    summary.key = "test-object";
    summary.size = 1024;
    std::vector<ObjectSummary> objects = {summary};

    ListBucketResultXml body;
    body.name = "test-bucket";
    body.encodingType = "url";
    body.nextMarker = "next-marker";
    body.prefix = "test-prefix";
    body.marker = "current-marker";
    body.delimiter = "/";
    body.maxKeys = 100;
    body.isTruncated = true;
    body.contents = objects;
    body.commonPrefixes = prefixes;

    auto result = ListObjectsResult(200, {{"x-oss-request-id", "id-123"}}, body);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("test-bucket", result.getName());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("next-marker", result.getNextMarker());
    EXPECT_EQ("test-prefix", result.getPrefix());
    EXPECT_EQ("current-marker", result.getMarker());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(100, result.getMaxKeys());
    EXPECT_TRUE(result.getIsTruncated());
    EXPECT_EQ(1, result.getContents().size());
    EXPECT_EQ(1, result.getCommonPrefixes().size());
    EXPECT_EQ("test-object", result.getContents()[0].key);
    EXPECT_EQ("test-prefix/", result.getCommonPrefixes()[0].prefix);
}

TEST(BucketBasicTest, ListObjectsV2Request_ConstructorDefault) {
    // Default
    auto request = ListObjectsV2Request();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(-1, request.getMaxKeys());
    EXPECT_EQ(false, request.getFetchOwner());
    EXPECT_EQ("", request.getStartAfter());
    EXPECT_EQ("", request.getContinuationToken());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, ListObjectsV2Request_Setter) {
    auto request = ListObjectsV2Request();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ(-1, request.getMaxKeys());

    // Setter
    request.setBucket("test-bucket")
            .setDelimiter("/")
            .setMaxKeys(100)
            .setPrefix("prefix-")
            .setEncodingType("url")
            .setFetchOwner(true)
            .setStartAfter("start-after-key")
            .setContinuationToken("token-123");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("/", request.getDelimiter());
    EXPECT_EQ(100, request.getMaxKeys());
    EXPECT_EQ("prefix-", request.getPrefix());
    EXPECT_EQ("url", request.getEncodingType());
    EXPECT_EQ(true, request.getFetchOwner());
    EXPECT_EQ("start-after-key", request.getStartAfter());
    EXPECT_EQ("token-123", request.getContinuationToken());
}

TEST(BucketBasicTest, ListObjectsV2Result_ConstructorDefault) {
    auto result = ListObjectsV2Result();
    EXPECT_EQ("", result.getName());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("", result.getContinuationToken());
    EXPECT_EQ("", result.getNextContinuationToken());
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getStartAfter());
    EXPECT_EQ("", result.getDelimiter());
    EXPECT_EQ(-1, result.getMaxKeys());
    EXPECT_EQ(-1, result.getKeyCount());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(0, result.getContents().size());
    EXPECT_EQ(0, result.getCommonPrefixes().size());
}

TEST(BucketBasicTest, ListObjectsV2Result_ConstructorAll) {
    CommonPrefix prefix;
    prefix.prefix = "test-prefix-v2/";
    std::vector<CommonPrefix> prefixes = {prefix};

    ObjectSummary summary;
    summary.key = "test-object-v2";
    summary.size = 2048;
    std::vector<ObjectSummary> objects = {summary};

    ListBucketResultXml body;
    body.name = "test-bucket-v2";
    body.encodingType = "url";
    body.continuationToken = "cont-token";
    body.nextContinuationToken = "next-cont-token";
    body.startAfter = "start-after";
    body.prefix = "test-prefix-v2";
    body.delimiter = "/";
    body.maxKeys = 500;
    body.isTruncated = true;
    body.keyCount = 450;
    body.contents = objects;
    body.commonPrefixes = prefixes;

    auto result = ListObjectsV2Result(200, {{"x-oss-request-id", "id-v2-123"}}, body);
    EXPECT_EQ("id-v2-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("test-bucket-v2", result.getName());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("cont-token", result.getContinuationToken());
    EXPECT_EQ("next-cont-token", result.getNextContinuationToken());
    EXPECT_EQ("start-after", result.getStartAfter());
    EXPECT_EQ("test-prefix-v2", result.getPrefix());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(500, result.getMaxKeys());
    EXPECT_EQ(450, result.getKeyCount());
    EXPECT_EQ(true, result.getIsTruncated());
    EXPECT_EQ(1, result.getContents().size());
    EXPECT_EQ(1, result.getCommonPrefixes().size());
    EXPECT_EQ("test-object-v2", result.getContents()[0].key);
    EXPECT_EQ("test-prefix-v2/", result.getCommonPrefixes()[0].prefix);
}

TEST(BucketBasicTest, GetBucketInfoRequest_ConstructorDefault) {
    // Default
    auto request = GetBucketInfoRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, GetBucketInfoRequest_Setter) {
    auto request = GetBucketInfoRequest();
    EXPECT_EQ("", request.getBucket());

    // Setter
    request.setBucket("test-bucket");
    EXPECT_EQ("test-bucket", request.getBucket());
}

TEST(BucketBasicTest, GetBucketInfoResult_ConstructorDefault) {
    auto result = GetBucketInfoResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasBucketInfo());
}

TEST(BucketBasicTest, GetBucketInfoResult_ConstructorAll) {
    auto result = GetBucketInfoResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasBucketInfo());
}

TEST(BucketBasicTest, GetBucketInfoResult_SetBody) {
    auto result = GetBucketInfoResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});

    BucketInfo info;
    info.setName("test-bucket").setLocation("oss-cn-hangzhou").setStorageClass("Standard");
    result.setBucketInfo(info);

    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasBucketInfo());
    EXPECT_EQ("test-bucket", result.getBucketInfo().name);
    EXPECT_EQ("oss-cn-hangzhou", result.getBucketInfo().location);
    EXPECT_EQ("Standard", result.getBucketInfo().storageClass);
}

TEST(BucketBasicTest, GetBucketLocationRequest_ConstructorDefault) {
    // Default
    auto request = GetBucketLocationRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketBasicTest, GetBucketLocationRequest_Setter) {
    auto request = GetBucketLocationRequest();
    EXPECT_EQ("", request.getBucket());

    // Setter
    request.setBucket("test-bucket");
    EXPECT_EQ("test-bucket", request.getBucket());
}

TEST(BucketBasicTest, GetBucketLocationResult_ConstructorDefault) {
    auto result = GetBucketLocationResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ("", result.getLocationConstraint());
}

TEST(BucketBasicTest, GetBucketLocationResult_ConstructorAll) {
    auto result = GetBucketLocationResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ("", result.getLocationConstraint());
}

TEST(BucketBasicTest, GetBucketLocationResult_SetLocationConstraint) {
    auto result = GetBucketLocationResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    result.setLocationConstraint("oss-cn-hangzhou");
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ("oss-cn-hangzhou", result.getLocationConstraint());
}

} // namespace models
} // namespace oss2
} // namespace alibabacloud