#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

// Test JobParameters struct
TEST(ObjectBasicTest, JobParameters_ConstructorDefault) {
    auto params = JobParameters();
    EXPECT_FALSE(params.tier.has_value());
}

TEST(ObjectBasicTest, JobParameters_Setter) {
    auto params = JobParameters();
    EXPECT_FALSE(params.tier.has_value());

    // Setter
    params.setTier("Expedited");

    EXPECT_TRUE(params.tier.has_value());
    EXPECT_EQ("Expedited", params.tier.value());
}

// Test RestoreRequest struct
TEST(ObjectBasicTest, RestoreRequest_ConstructorDefault) {
    auto request = RestoreRequest();
    EXPECT_FALSE(request.days.has_value());
    EXPECT_FALSE(request.jobParameters.has_value());
}

TEST(ObjectBasicTest, RestoreRequest_Setter) {
    auto request = RestoreRequest();
    EXPECT_FALSE(request.days.has_value());
    EXPECT_FALSE(request.jobParameters.has_value());

    // Setter
    request.setDays(5);

    JobParameters jobParams;
    jobParams.setTier("Standard");
    request.setJobParameters(jobParams);

    EXPECT_TRUE(request.days.has_value());
    EXPECT_EQ(5, request.days.value());
    EXPECT_TRUE(request.jobParameters.has_value());
    EXPECT_EQ("Standard", request.jobParameters.value().tier.value());
}

// Test CopyObjectResultXml struct
TEST(ObjectBasicTest, CopyObjectResultXml_ConstructorDefault) {
    auto resultXml = CopyObjectResultXml();
    EXPECT_EQ("", resultXml.lastModified);
    EXPECT_EQ("", resultXml.eTag);
}

TEST(ObjectBasicTest, CopyObjectResultXml_Setter) {
    auto resultXml = CopyObjectResultXml();
    EXPECT_EQ("", resultXml.lastModified);
    EXPECT_EQ("", resultXml.eTag);

    // Direct assignment (since CopyObjectResultXml has public members)
    resultXml.lastModified = "2023-01-01T00:00:00Z";
    resultXml.eTag = "test-etag";

    EXPECT_EQ("2023-01-01T00:00:00Z", resultXml.lastModified);
    EXPECT_EQ("test-etag", resultXml.eTag);
}

// Test PutObjectRequest
TEST(ObjectBasicTest, PutObjectRequest_ConstructorDefault) {
    auto request = PutObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getServerSideEncryption());
    EXPECT_EQ("", request.getServerSideDataEncryption());
    EXPECT_EQ("", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("", request.getObjectAcl());
    EXPECT_EQ("", request.getStorageClass());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ(0, request.getMetadata().size());
    EXPECT_FALSE(request.hasBody());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, PutObjectRequest_Setter) {
    auto request = PutObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setForbidOverwrite("true");
    request.setServerSideEncryption("AES256");
    request.setServerSideDataEncryption("SM4");
    request.setServerSideEncryptionKeyId("key-id");
    request.setObjectAcl("private");
    request.setStorageClass("Standard");
    request.setTagging("tag1=value1");

    HeaderCollection metadata;
    metadata["custom-header"] = "custom-value";
    request.setMetadata(metadata);

    auto content = std::make_shared<StringContent>("test data");
    request.setBody(content);

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("true", request.getForbidOverwrite());
    EXPECT_EQ("AES256", request.getServerSideEncryption());
    EXPECT_EQ("SM4", request.getServerSideDataEncryption());
    EXPECT_EQ("key-id", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("private", request.getObjectAcl());
    EXPECT_EQ("Standard", request.getStorageClass());
    EXPECT_EQ("tag1=value1", request.getTagging());
    EXPECT_EQ("custom-value", request.getMetadata().at("custom-header"));
    EXPECT_TRUE(request.hasBody());
}

TEST(ObjectBasicTest, PutObjectResult_ConstructorDefault) {
    auto result = PutObjectResult();
    EXPECT_EQ("", result.getHashCrc64ecma());
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, PutObjectResult_ConstructorAll) {
    auto result = PutObjectResult(200, {{"x-oss-hash-crc64ecma", "123456789"},
                                        {"x-oss-version-id", "version123"},
                                        {"x-oss-request-id", "req123"}});
    EXPECT_EQ("123456789", result.getHashCrc64ecma());
    EXPECT_EQ("version123", result.getVersionId());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());
}

// Test CopyObjectRequest
TEST(ObjectBasicTest, CopyObjectRequest_ConstructorDefault) {
    auto request = CopyObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getCopySource());
    EXPECT_EQ("", request.getSourceBucket());
    EXPECT_EQ("", request.getSourceKey());
    EXPECT_EQ("", request.getSourceVersiondId());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getCopySourceIfMatch());
    EXPECT_EQ("", request.getCopySourceIfNoneMatch());
    EXPECT_EQ("", request.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("", request.getCopySourceIfModifiedSince());
    EXPECT_EQ("", request.getMetadataDirective());
    EXPECT_EQ("", request.getServerSideEncryption());
    EXPECT_EQ("", request.getServerSideDataEncryption());
    EXPECT_EQ("", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("", request.getObjectAcl());
    EXPECT_EQ("", request.getStorageClass());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ("", request.getTaggingDirective());
    EXPECT_EQ(0, request.getMetadata().size());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, CopyObjectRequest_Setter) {
    auto request = CopyObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getSourceBucket());
    EXPECT_EQ("", request.getSourceKey());

    // Setter
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setSourceBucket("src-bucket");
    request.setSourceKey("src-key");
    request.setourceVersiondId("version-id");
    request.setCopySource("/src-bucket/src-key");
    request.setForbidOverwrite("false");
    request.setCopySourceIfMatch("etag123");
    request.setCopySourceIfNoneMatch("etag456");
    request.setCopySourceIfUnmodifiedSince("2023-01-01T00:00:00Z");
    request.setCopySourceIfModifiedSince("2023-01-02T00:00:00Z");
    request.setMetadataDirective("REPLACE");
    request.setServerSideEncryption("AES256");
    request.setServerSideDataEncryption("SM4");
    request.setServerSideEncryptionKeyId("key-id");
    request.setObjectAcl("public-read");
    request.setStorageClass("IA");
    request.setTagging("tag1=value1");
    request.setTaggingDirective("Replace");

    HeaderCollection metadata;
    metadata["custom-meta"] = "custom-value";
    request.setMetadata(metadata);

    EXPECT_EQ("dest-bucket", request.getBucket());
    EXPECT_EQ("dest-key", request.getKey());
    EXPECT_EQ("/src-bucket/src-key", request.getCopySource());
    EXPECT_EQ("src-bucket", request.getSourceBucket());
    EXPECT_EQ("src-key", request.getSourceKey());
    EXPECT_EQ("version-id", request.getSourceVersiondId());
    EXPECT_EQ("false", request.getForbidOverwrite());
    EXPECT_EQ("etag123", request.getCopySourceIfMatch());
    EXPECT_EQ("etag456", request.getCopySourceIfNoneMatch());
    EXPECT_EQ("2023-01-01T00:00:00Z", request.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("2023-01-02T00:00:00Z", request.getCopySourceIfModifiedSince());
    EXPECT_EQ("REPLACE", request.getMetadataDirective());
    EXPECT_EQ("AES256", request.getServerSideEncryption());
    EXPECT_EQ("SM4", request.getServerSideDataEncryption());
    EXPECT_EQ("key-id", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("public-read", request.getObjectAcl());
    EXPECT_EQ("IA", request.getStorageClass());
    EXPECT_EQ("tag1=value1", request.getTagging());
    EXPECT_EQ("Replace", request.getTaggingDirective());
    EXPECT_EQ("custom-value", request.getMetadata().at("custom-meta"));
}

TEST(ObjectBasicTest, CopyObjectResult_ConstructorDefault) {
    auto result = CopyObjectResult();
    EXPECT_EQ("", result.getCopySourceVersionId());
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getLastModified());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, CopyObjectResult_ConstructorAll) {
    CopyObjectResultXml xmlBody;
    xmlBody.eTag = "test-etag";
    xmlBody.lastModified = "2023-01-01T00:00:00Z";

    auto result = CopyObjectResult(200,
                                   {{"x-oss-copy-source-version-id", "src-version123"},
                                    {"x-oss-version-id", "version123"},
                                    {"x-oss-request-id", "req123"}},
                                   xmlBody);
    EXPECT_EQ("src-version123", result.getCopySourceVersionId());
    EXPECT_EQ("version123", result.getVersionId());
    EXPECT_EQ("test-etag", result.getETag());
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastModified());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());
}

// Test GetObjectRequest
TEST(ObjectBasicTest, GetObjectRequest_ConstructorDefault) {
    auto request = GetObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getRange());
    EXPECT_EQ("", request.getIfModifiedSince());
    EXPECT_EQ("", request.getIfUnmodifiedSince());
    EXPECT_EQ("", request.getIfMatch());
    EXPECT_EQ("", request.getIfNoneMatch());
    EXPECT_EQ("", request.getAcceptEncoding());
    EXPECT_EQ("", request.getResponseContentType());
    EXPECT_EQ("", request.getResponseContentLanguage());
    EXPECT_EQ("", request.getResponseExpires());
    EXPECT_EQ("", request.getResponseCacheControl());
    EXPECT_EQ("", request.getResponseContentDisposition());
    EXPECT_EQ("", request.getResponseContentEncoding());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, GetObjectRequest_Setter) {
    auto request = GetObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setRange("bytes=0-1023");
    request.setIfModifiedSince("2023-01-01T00:00:00Z");
    request.setIfUnmodifiedSince("2023-01-02T00:00:00Z");
    request.setIfMatch("etag123");
    request.setIfNoneMatch("etag456");
    request.setAcceptEncoding("gzip");
    request.setResponseContentType("text/plain");
    request.setResponseContentLanguage("en-US");
    request.setResponseExpires("2024-01-01T00:00:00Z");
    request.setResponseCacheControl("no-cache");
    request.setResponseContentDisposition("attachment; filename=test.txt");
    request.setResponseContentEncoding("gzip");
    request.setVersionId("version123");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("bytes=0-1023", request.getRange());
    EXPECT_EQ("2023-01-01T00:00:00Z", request.getIfModifiedSince());
    EXPECT_EQ("2023-01-02T00:00:00Z", request.getIfUnmodifiedSince());
    EXPECT_EQ("etag123", request.getIfMatch());
    EXPECT_EQ("etag456", request.getIfNoneMatch());
    EXPECT_EQ("gzip", request.getAcceptEncoding());
    EXPECT_EQ("text/plain", request.getResponseContentType());
    EXPECT_EQ("en-US", request.getResponseContentLanguage());
    EXPECT_EQ("2024-01-01T00:00:00Z", request.getResponseExpires());
    EXPECT_EQ("no-cache", request.getResponseCacheControl());
    EXPECT_EQ("attachment; filename=test.txt", request.getResponseContentDisposition());
    EXPECT_EQ("gzip", request.getResponseContentEncoding());
    EXPECT_EQ("version123", request.getVersionId());
}

TEST(ObjectBasicTest, GetObjectResult_ConstructorDefault) {
    auto result = GetObjectResult();
    EXPECT_EQ("", result.getContentMd5());
    EXPECT_EQ("", result.getLastModified());
    EXPECT_EQ("", result.getExpiration());
    EXPECT_EQ(-1, result.getTaggingCount());
    EXPECT_EQ("", result.getContentType());
    EXPECT_EQ(-1, result.getNextAppendPosition());
    EXPECT_EQ("", result.getHashCrc64ecma());
    EXPECT_EQ("", result.getServerSideEncryptionKeyId());
    EXPECT_EQ("", result.getObjectType());
    EXPECT_EQ("", result.getRequestCharged());
    EXPECT_EQ(-1, result.getContentLength());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ(0, result.getMetadata().size());
    EXPECT_EQ("", result.getServerSideEncryption());
    EXPECT_EQ("", result.getStorageClass());
    EXPECT_EQ("", result.getRestore());
    EXPECT_EQ("", result.getProcessStatus());
    EXPECT_EQ(nullptr, result.getBody());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, GetObjectResult_ConstructorAll) {
    auto body = std::make_shared<std::stringstream>("test content");
    auto result = GetObjectResult(200,
                                  {{"Content-Md5", "md5-123"},
                                   {"Last-Modified", "2023-01-01T00:00:00Z"},
                                   {"x-oss-expiration", "2024-01-01T00:00:00Z"},
                                   {"x-oss-tagging-count", "2"},
                                   {"Content-Type", "text/plain"},
                                   {"x-oss-next-append-position", "1024"},
                                   {"x-oss-hash-crc64ecma", "crc123"},
                                   {"x-oss-server-side-encryption-key-id", "key123"},
                                   {"x-oss-object-type", "Normal"},
                                   {"x-oss-request-charged", "true"},
                                   {"Content-Length", "1024"},
                                   {"ETag", "etag123"},
                                   {"x-oss-server-side-encryption", "AES256"},
                                   {"x-oss-storage-class", "Standard"},
                                   {"x-oss-restore", "ongoing-request=true"},
                                   {"x-oss-process-status", "normal"},
                                   {"x-oss-request-id", "req123"}},
                                  body);

    EXPECT_EQ("md5-123", result.getContentMd5());
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastModified());
    EXPECT_EQ("2024-01-01T00:00:00Z", result.getExpiration());
    EXPECT_EQ(2, result.getTaggingCount());
    EXPECT_EQ("text/plain", result.getContentType());
    EXPECT_EQ(1024, result.getNextAppendPosition());
    EXPECT_EQ("crc123", result.getHashCrc64ecma());
    EXPECT_EQ("key123", result.getServerSideEncryptionKeyId());
    EXPECT_EQ("Normal", result.getObjectType());
    EXPECT_EQ("true", result.getRequestCharged());
    EXPECT_EQ(1024, result.getContentLength());
    EXPECT_EQ("etag123", result.getETag());
    EXPECT_EQ("AES256", result.getServerSideEncryption());
    EXPECT_EQ("Standard", result.getStorageClass());
    EXPECT_EQ("ongoing-request=true", result.getRestore());
    EXPECT_EQ("normal", result.getProcessStatus());
    EXPECT_EQ(body, result.getBody());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(17, result.getHeaders().size());
}

// Test AppendObjectRequest
TEST(ObjectBasicTest, AppendObjectRequest_ConstructorDefault) {
    auto request = AppendObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getServerSideEncryption());
    EXPECT_EQ("", request.getObjectAcl());
    EXPECT_EQ("", request.getStorageClass());
    EXPECT_EQ(0, request.getMetadata().size());
    EXPECT_EQ("", request.getCacheControl());
    EXPECT_EQ("", request.getContentDisposition());
    EXPECT_EQ("", request.getContentEncoding());
    EXPECT_EQ("", request.getContentMd5());
    EXPECT_EQ("", request.getExpires());
    EXPECT_EQ(-1, request.getPosition());
    EXPECT_FALSE(request.hasBody());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, AppendObjectRequest_Setter) {
    auto request = AppendObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setServerSideEncryption("AES256");
    request.setObjectAcl("private");
    request.setStorageClass("Standard");

    HeaderCollection metadata;
    metadata["custom-meta"] = "custom-value";
    request.setMetadata(metadata);

    request.setCacheControl("no-cache");
    request.setContentDisposition("attachment; filename=test.txt");
    request.setContentEncoding("gzip");
    request.setContentMd5("md5-123");
    request.setExpires("2024-01-01T00:00:00Z");
    request.setPosition(1024);

    auto content = std::make_shared<StringContent>("test data");
    request.setBody(content);

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("AES256", request.getServerSideEncryption());
    EXPECT_EQ("private", request.getObjectAcl());
    EXPECT_EQ("Standard", request.getStorageClass());
    EXPECT_EQ("custom-value", request.getMetadata().at("custom-meta"));
    EXPECT_EQ("no-cache", request.getCacheControl());
    EXPECT_EQ("attachment; filename=test.txt", request.getContentDisposition());
    EXPECT_EQ("gzip", request.getContentEncoding());
    EXPECT_EQ("md5-123", request.getContentMd5());
    EXPECT_EQ("2024-01-01T00:00:00Z", request.getExpires());
    EXPECT_EQ(1024, request.getPosition());
    EXPECT_TRUE(request.hasBody());
}

TEST(ObjectBasicTest, AppendObjectResult_ConstructorDefault) {
    auto result = AppendObjectResult();
    EXPECT_EQ(-1, result.getNextAppendPosition());
    EXPECT_EQ("", result.getHashCrc64ecma());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, AppendObjectResult_ConstructorAll) {
    auto result = AppendObjectResult(200, {{"x-oss-next-append-position", "2048"},
                                           {"x-oss-hash-crc64ecma", "crc123"},
                                           {"x-oss-request-id", "req123"}});
    EXPECT_EQ(2048, result.getNextAppendPosition());
    EXPECT_EQ("crc123", result.getHashCrc64ecma());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());
}

// Test SealAppendObjectRequest
TEST(ObjectBasicTest, SealAppendObjectRequest_ConstructorDefault) {
    auto request = SealAppendObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ(-1, request.getPosition());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, SealAppendObjectRequest_Setter) {
    auto request = SealAppendObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ(-1, request.getPosition());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPosition(2048);

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ(2048, request.getPosition());
}

TEST(ObjectBasicTest, SealAppendObjectResult_ConstructorDefault) {
    auto result = SealAppendObjectResult();
    EXPECT_EQ("", result.getSealedTime());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, SealAppendObjectResult_ConstructorAll) {
    auto result = SealAppendObjectResult(
            200, {{"x-oss-sealed-time", "2023-01-01T00:00:00Z"}, {"x-oss-request-id", "req123"}});
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getSealedTime());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}

// Test DeleteObjectRequest
TEST(ObjectBasicTest, DeleteObjectRequest_ConstructorDefault) {
    auto request = DeleteObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, DeleteObjectRequest_Setter) {
    auto request = DeleteObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setVersionId("version123");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("version123", request.getVersionId());
}

TEST(ObjectBasicTest, DeleteObjectResult_ConstructorDefault) {
    auto result = DeleteObjectResult();
    EXPECT_EQ("", result.getDeleteMarker());
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, DeleteObjectResult_ConstructorAll) {
    auto result = DeleteObjectResult(
            204, {{"x-oss-delete-marker", "true"}, {"x-oss-version-id", "version123"}, {"x-oss-request-id", "req123"}});
    EXPECT_EQ("true", result.getDeleteMarker());
    EXPECT_EQ("version123", result.getVersionId());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());
}

// Test HeadObjectRequest
TEST(ObjectBasicTest, HeadObjectRequest_ConstructorDefault) {
    auto request = HeadObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getIfModifiedSince());
    EXPECT_EQ("", request.getIfUnmodifiedSince());
    EXPECT_EQ("", request.getIfMatch());
    EXPECT_EQ("", request.getIfNoneMatch());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, HeadObjectRequest_Setter) {
    auto request = HeadObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setIfModifiedSince("2023-01-01T00:00:00Z");
    request.setIfUnmodifiedSince("2023-01-02T00:00:00Z");
    request.setIfMatch("etag123");
    request.setIfNoneMatch("etag456");
    request.setVersionId("version123");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("2023-01-01T00:00:00Z", request.getIfModifiedSince());
    EXPECT_EQ("2023-01-02T00:00:00Z", request.getIfUnmodifiedSince());
    EXPECT_EQ("etag123", request.getIfMatch());
    EXPECT_EQ("etag456", request.getIfNoneMatch());
    EXPECT_EQ("version123", request.getVersionId());
}

TEST(ObjectBasicTest, HeadObjectResult_ConstructorDefault) {
    auto result = HeadObjectResult();
    EXPECT_EQ("", result.getProcessStatus());
    EXPECT_EQ("", result.getRequestCharged());
    EXPECT_EQ("", result.getContentType());
    EXPECT_EQ("", result.getServerSideEncryption());
    EXPECT_EQ("", result.getObjectType());
    EXPECT_EQ("", result.getExpiration());
    EXPECT_EQ("", result.getContentMd5());
    EXPECT_EQ(-1, result.getContentLength());
    EXPECT_EQ("", result.getLastModified());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getServerSideEncryptionKeyId());
    EXPECT_EQ(-1, result.getNextAppendPosition());
    EXPECT_EQ("", result.getRestore());
    EXPECT_EQ("", result.getTransitionTime());
    EXPECT_EQ(-1, result.getTaggingCount());
    EXPECT_EQ("", result.getHashCrc64ecma());
    EXPECT_EQ(0, result.getMetadata().size());
    EXPECT_EQ("", result.getStorageClass());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, HeadObjectResult_ConstructorAll) {
    auto result = HeadObjectResult(200, {{"x-oss-process-status", "normal"},
                                         {"x-oss-request-charged", "true"},
                                         {"Content-Type", "text/plain"},
                                         {"x-oss-server-side-encryption", "AES256"},
                                         {"x-oss-object-type", "Normal"},
                                         {"x-oss-expiration", "2024-01-01T00:00:00Z"},
                                         {"Content-Md5", "md5-123"},
                                         {"Content-Length", "1024"},
                                         {"Last-Modified", "2023-01-01T00:00:00Z"},
                                         {"ETag", "etag123"},
                                         {"x-oss-server-side-encryption-key-id", "key123"},
                                         {"x-oss-next-append-position", "2048"},
                                         {"x-oss-restore", "ongoing-request=true"},
                                         {"x-oss-transition-time", "2023-02-01T00:00:00Z"},
                                         {"x-oss-tagging-count", "2"},
                                         {"x-oss-hash-crc64ecma", "crc123"},
                                         {"x-oss-storage-class", "Standard"},
                                         {"x-oss-request-id", "req123"}});

    EXPECT_EQ("normal", result.getProcessStatus());
    EXPECT_EQ("true", result.getRequestCharged());
    EXPECT_EQ("text/plain", result.getContentType());
    EXPECT_EQ("AES256", result.getServerSideEncryption());
    EXPECT_EQ("Normal", result.getObjectType());
    EXPECT_EQ("2024-01-01T00:00:00Z", result.getExpiration());
    EXPECT_EQ("md5-123", result.getContentMd5());
    EXPECT_EQ(1024, result.getContentLength());
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastModified());
    EXPECT_EQ("etag123", result.getETag());
    EXPECT_EQ("key123", result.getServerSideEncryptionKeyId());
    EXPECT_EQ(2048, result.getNextAppendPosition());
    EXPECT_EQ("ongoing-request=true", result.getRestore());
    EXPECT_EQ("2023-02-01T00:00:00Z", result.getTransitionTime());
    EXPECT_EQ(2, result.getTaggingCount());
    EXPECT_EQ("crc123", result.getHashCrc64ecma());
    EXPECT_EQ("Standard", result.getStorageClass());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(18, result.getHeaders().size());
}

// Test GetObjectMetaRequest
TEST(ObjectBasicTest, GetObjectMetaRequest_ConstructorDefault) {
    auto request = GetObjectMetaRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, GetObjectMetaRequest_Setter) {
    auto request = GetObjectMetaRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setVersionId("version123");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("version123", request.getVersionId());
}

TEST(ObjectBasicTest, GetObjectMetaResult_ConstructorDefault) {
    auto result = GetObjectMetaResult();
    EXPECT_EQ("", result.getLastAccessTime());
    EXPECT_EQ("", result.getLastModifed());
    EXPECT_EQ("", result.getTransitionTime());
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ(-1, result.getContentLength());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, GetObjectMetaResult_ConstructorAll) {
    auto result = GetObjectMetaResult(200, {{"x-oss-last-access-time", "2023-01-01T00:00:00Z"},
                                            {"Last-Modifed", "2023-01-01T00:00:00Z"},
                                            {"x-oss-transition-time", "2023-02-01T00:00:00Z"},
                                            {"x-oss-version-id", "version123"},
                                            {"ETag", "etag123"},
                                            {"Content-Length", "1024"},
                                            {"x-oss-request-id", "req123"}});

    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastAccessTime());
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastModifed());
    EXPECT_EQ("2023-02-01T00:00:00Z", result.getTransitionTime());
    EXPECT_EQ("version123", result.getVersionId());
    EXPECT_EQ("etag123", result.getETag());
    EXPECT_EQ(1024, result.getContentLength());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(7, result.getHeaders().size());
}

// Test RestoreObjectRequest
TEST(ObjectBasicTest, RestoreObjectRequest_ConstructorDefault) {
    auto request = RestoreObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_FALSE(request.hasRestoreRequest());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, RestoreObjectRequest_Setter) {
    auto request = RestoreObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_FALSE(request.hasRestoreRequest());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setVersionId("version123");

    RestoreRequest restoreReq;
    restoreReq.setDays(5);
    JobParameters jobParams;
    jobParams.setTier("Standard");
    restoreReq.setJobParameters(jobParams);

    request.setRestoreRequest(restoreReq);

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("version123", request.getVersionId());
    EXPECT_TRUE(request.hasRestoreRequest());

    auto restoreReqFromRequest = request.getRestoreRequest();
    EXPECT_TRUE(restoreReqFromRequest.days.has_value());
    EXPECT_EQ(5, restoreReqFromRequest.days.value());
    EXPECT_TRUE(restoreReqFromRequest.jobParameters.has_value());
    EXPECT_TRUE(restoreReqFromRequest.jobParameters.value().tier.has_value());
    EXPECT_EQ("Standard", restoreReqFromRequest.jobParameters.value().tier.value());
}

TEST(ObjectBasicTest, RestoreObjectResult_ConstructorDefault) {
    auto result = RestoreObjectResult();
    EXPECT_EQ("", result.getObjectRestorePriority());
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, RestoreObjectResult_ConstructorAll) {
    auto result = RestoreObjectResult(200, {{"x-oss-object-restore-priority", "Standard"},
                                            {"x-oss-version-id", "version123"},
                                            {"x-oss-request-id", "req123"}});
    EXPECT_EQ("Standard", result.getObjectRestorePriority());
    EXPECT_EQ("version123", result.getVersionId());
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());
}

// Test CleanRestoredObjectRequest
TEST(ObjectBasicTest, CleanRestoredObjectRequest_ConstructorDefault) {
    auto request = CleanRestoredObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectBasicTest, CleanRestoredObjectRequest_Setter) {
    auto request = CleanRestoredObjectRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
}

TEST(ObjectBasicTest, CleanRestoredObjectResult_ConstructorDefault) {
    auto result = CleanRestoredObjectResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectBasicTest, CleanRestoredObjectResult_ConstructorAll) {
    auto result = CleanRestoredObjectResult(204, {{"x-oss-request-id", "req123"}});
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());
}

// Test ObjectIdentifier struct
TEST(ObjectBasicTest, ObjectIdentifier_ConstructorDefault) {
    auto params = ObjectIdentifier();
    EXPECT_EQ("", params.key);
    EXPECT_FALSE(params.versionId.has_value());
}

TEST(ObjectBasicTest, ObjectIdentifier_Setter) {
    auto params = ObjectIdentifier();
    EXPECT_EQ("", params.key);
    EXPECT_FALSE(params.versionId.has_value());

    // Setter
    params.setKey("key");
    params.setVersionId("id-123");

    EXPECT_EQ("key", params.key);
    EXPECT_EQ("id-123", params.versionId);
}

// Test Delete struct
TEST(ObjectBasicTest, Delete_ConstructorDefault) {
    auto params = Delete();
    EXPECT_EQ(0, params.objects.size());
    EXPECT_FALSE(params.quiet.has_value());
}


TEST(ObjectBasicTest, Delete_Setter) {
    auto params = Delete();
    EXPECT_EQ(0, params.objects.size());
    EXPECT_FALSE(params.quiet.has_value());

    // Setter
    params.setObjects({ObjectIdentifier{"key1"}, {ObjectIdentifier{"key2", "id-2"}}});
    params.setQuiet(true);

    EXPECT_EQ(2, params.objects.size());
    EXPECT_EQ("key1", params.objects.at(0).key);
    EXPECT_FALSE(params.objects.at(0).versionId);

    EXPECT_EQ("key2", params.objects.at(1).key);
    EXPECT_EQ("id-2", params.objects.at(1).versionId);
}

// Test DeletedInfo struct
TEST(ObjectBasicTest, DeletedInfo_ConstructorDefault) {
    auto params = DeletedInfo();
    EXPECT_EQ("", params.key);
    EXPECT_FALSE(params.versionId.has_value());
    EXPECT_FALSE(params.deleteMarker.has_value());
    EXPECT_FALSE(params.deleteMarkerVersionId.has_value());
}


TEST(ObjectBasicTest, DeletedInfo_Setter) {
    auto params = DeletedInfo();
    EXPECT_EQ("", params.key);
    EXPECT_FALSE(params.versionId.has_value());
    EXPECT_FALSE(params.deleteMarker.has_value());
    EXPECT_FALSE(params.deleteMarkerVersionId.has_value());

    // Setter
    params.setKey("key");
    params.setVersionId("id-123");
    params.setDeleteMarker(true);
    params.setDeleteMarkerVersionId("1i-1234");

    EXPECT_EQ("key", params.key);
    EXPECT_EQ("id-123", params.versionId);
    EXPECT_EQ(true, params.deleteMarker);
    EXPECT_EQ("1i-1234", params.deleteMarkerVersionId);
}

// Test DeleteMultipleObjectsRequest
TEST(ObjectBasicTest, DeleteMultipleObjectsRequest_ConstructorDefault) {
    auto request = DeleteMultipleObjectsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(false, request.hasDelete());
}

TEST(ObjectBasicTest, DeleteMultipleObjectsRequest_Setter) {
    auto request = DeleteMultipleObjectsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(false, request.hasDelete());

    // Setter
    request.setBucket("test-bucket");
    request.setDelete(Delete{{ObjectIdentifier{"key1"}, {ObjectIdentifier{"key2", "id-2"}}}});
    request.setEncodingType("url");

    EXPECT_EQ(true, request.hasDelete());
    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("url", request.getEncodingType());
    EXPECT_EQ(2, request.getDelete().objects.size());
    EXPECT_EQ("key1", request.getDelete().objects.at(0).key);
    EXPECT_FALSE(request.getDelete().objects.at(0).versionId.has_value());

    EXPECT_EQ("key2", request.getDelete().objects.at(1).key);
    EXPECT_EQ("id-2", request.getDelete().objects.at(1).versionId);
}

TEST(ObjectBasicTest, DeleteMultipleObjectsResult_ConstructorDefault) {
    auto result = DeleteMultipleObjectsResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());

    EXPECT_EQ(0, result.getDeletedObjects().size());
    EXPECT_EQ("", result.getEncodingType());
}

TEST(ObjectBasicTest, DeleteMultipleObjectsResult_ConstructorAll) {
    auto result = DeleteMultipleObjectsResult(204, {{"x-oss-request-id", "req123"}});
    EXPECT_EQ("req123", result.getRequestId());
    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());

    result.setDeletedObjects({DeletedInfo{"key"}});
    result.setEncodingType("url");
    EXPECT_EQ(1, result.getDeletedObjects().size());
    EXPECT_EQ("url", result.getEncodingType());
}

} // namespace models
} // namespace oss2
} // namespace alibabacloud