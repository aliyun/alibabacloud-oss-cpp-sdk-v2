#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/ObjectMultipart.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

// Test Upload struct
TEST(ObjectMultipartTest, Upload_ConstructorDefault) {
    auto upload = Upload();
    EXPECT_EQ("", upload.key);
    EXPECT_EQ("", upload.uploadId);
    EXPECT_EQ("", upload.initiated);
}

TEST(ObjectMultipartTest, Upload_Setter) {
    auto upload = Upload();
    EXPECT_EQ("", upload.key);
    EXPECT_EQ("", upload.uploadId);
    EXPECT_EQ("", upload.initiated);

    // Setter
    upload.setKey("test-key");
    upload.setUploadId("test-upload-id");
    upload.setInitiated("2023-01-01T00:00:00Z");

    EXPECT_EQ("test-key", upload.key);
    EXPECT_EQ("test-upload-id", upload.uploadId);
    EXPECT_EQ("2023-01-01T00:00:00Z", upload.initiated);
}

// Test Part struct
TEST(ObjectMultipartTest, Part_ConstructorDefault) {
    auto part = Part();
    EXPECT_EQ("", part.eTag);
    EXPECT_EQ(0, part.partNumber);
    EXPECT_EQ(0, part.size);
    EXPECT_EQ("", part.lastModified);
}

TEST(ObjectMultipartTest, Part_Setter) {
    auto part = Part();
    EXPECT_EQ("", part.eTag);
    EXPECT_EQ(0, part.partNumber);
    EXPECT_EQ(0, part.size);
    EXPECT_EQ("", part.lastModified);

    // Setter
    part.setETag("test-etag");
    part.setPartNumber(1);
    part.setSize(1024);
    part.setLastModified("2023-01-01T00:00:00Z");

    EXPECT_EQ("test-etag", part.eTag);
    EXPECT_EQ(1, part.partNumber);
    EXPECT_EQ(1024, part.size);
    EXPECT_EQ("2023-01-01T00:00:00Z", part.lastModified);
}

// Test CompleteMultipartUpload struct
TEST(ObjectMultipartTest, CompleteMultipartUpload_ConstructorDefault) {
    auto complete = CompleteMultipartUpload();
    EXPECT_EQ(0, complete.parts.size());
}

TEST(ObjectMultipartTest, CompleteMultipartUpload_Setter) {
    auto complete = CompleteMultipartUpload();
    EXPECT_EQ(0, complete.parts.size());

    // Create test parts
    Part part1;
    part1.setETag("etag1").setPartNumber(1).setSize(1024).setLastModified("2023-01-01T00:00:00Z");

    Part part2;
    part2.setETag("etag2").setPartNumber(2).setSize(2048).setLastModified("2023-01-01T00:01:00Z");

    std::vector<Part> parts = {part1, part2};
    complete.setParts(parts);

    EXPECT_EQ(2, complete.parts.size());
    EXPECT_EQ("etag1", complete.parts[0].eTag);
    EXPECT_EQ(1, complete.parts[0].partNumber);
    EXPECT_EQ(1024, complete.parts[0].size);
    EXPECT_EQ("2023-01-01T00:00:00Z", complete.parts[0].lastModified);
    EXPECT_EQ("etag2", complete.parts[1].eTag);
    EXPECT_EQ(2, complete.parts[1].partNumber);
    EXPECT_EQ(2048, complete.parts[1].size);
    EXPECT_EQ("2023-01-01T00:01:00Z", complete.parts[1].lastModified);
}

// Test InitiateMultipartUploadRequest
TEST(ObjectMultipartTest, InitiateMultipartUploadRequest_ConstructorDefault) {
    auto request = InitiateMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getStorageClass());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ("", request.getServerSideEncryption());
    EXPECT_EQ("", request.getServerSideDataEncryption());
    EXPECT_EQ("", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("", request.getCacheControl());
    EXPECT_EQ("", request.getContentDisposition());
    EXPECT_EQ("", request.getContentEncoding());
    EXPECT_EQ("", request.getExpires());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, InitiateMultipartUploadRequest_Setter) {
    auto request = InitiateMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getStorageClass());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ("", request.getServerSideEncryption());
    EXPECT_EQ("", request.getServerSideDataEncryption());
    EXPECT_EQ("", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("", request.getCacheControl());
    EXPECT_EQ("", request.getContentDisposition());
    EXPECT_EQ("", request.getContentEncoding());
    EXPECT_EQ("", request.getExpires());
    EXPECT_EQ("", request.getEncodingType());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setForbidOverwrite("false");
    request.setStorageClass("Standard");
    request.setTagging("tag1=value1");
    request.setServerSideEncryption("AES256");
    request.setServerSideDataEncryption("SM4");
    request.setServerSideEncryptionKeyId("key-id");
    request.setCacheControl("no-cache");
    request.setContentDisposition("attachment; filename=test.txt");
    request.setContentEncoding("gzip");
    request.setExpires("Wed, 21 Oct 2023 07:28:00 GMT");
    request.setEncodingType("url");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("false", request.getForbidOverwrite());
    EXPECT_EQ("Standard", request.getStorageClass());
    EXPECT_EQ("tag1=value1", request.getTagging());
    EXPECT_EQ("AES256", request.getServerSideEncryption());
    EXPECT_EQ("SM4", request.getServerSideDataEncryption());
    EXPECT_EQ("key-id", request.getServerSideEncryptionKeyId());
    EXPECT_EQ("no-cache", request.getCacheControl());
    EXPECT_EQ("attachment; filename=test.txt", request.getContentDisposition());
    EXPECT_EQ("gzip", request.getContentEncoding());
    EXPECT_EQ("Wed, 21 Oct 2023 07:28:00 GMT", request.getExpires());
    EXPECT_EQ("url", request.getEncodingType());
}

// Test InitiateMultipartUploadResult
TEST(ObjectMultipartTest, InitiateMultipartUploadResult_ConstructorDefault) {
    auto result = InitiateMultipartUploadResult();
    EXPECT_EQ("", result.getKey());
    EXPECT_EQ("", result.getUploadId());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("", result.getBucket());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, InitiateMultipartUploadResult_ConstructorAll) {
    InitiateMultipartUploadResultXml xmlBody;
    xmlBody.bucket = "test-bucket";
    xmlBody.key = "test-key";
    xmlBody.uploadId = "test-upload-id";
    xmlBody.encodingType = "url";

    auto result = InitiateMultipartUploadResult(
            200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}}, xmlBody);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("test-bucket", result.getBucket());
}

// Test UploadPartRequest
TEST(ObjectMultipartTest, UploadPartRequest_ConstructorDefault) {
    auto request = UploadPartRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ(-1, request.getPartNumber());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_FALSE(request.hasBody());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, UploadPartRequest_Setter) {
    auto request = UploadPartRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ(-1, request.getPartNumber());
    EXPECT_EQ("", request.getUploadId());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setPartNumber(1);
    request.setUploadId("test-upload-id");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ(1, request.getPartNumber());
    EXPECT_EQ("test-upload-id", request.getUploadId());
}

// Test UploadPartResult
TEST(ObjectMultipartTest, UploadPartResult_ConstructorDefault) {
    auto result = UploadPartResult();
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getHashCrc64ecma());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, UploadPartResult_ConstructorAll) {
    auto result = UploadPartResult(200, {{"ETag", "test-etag"}, {"x-oss-hash-crc64ecma", "123456789"}});
    EXPECT_EQ("test-etag", result.getETag());
    EXPECT_EQ("123456789", result.getHashCrc64ecma());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}

// Test CompleteMultipartUploadRequest
TEST(ObjectMultipartTest, CompleteMultipartUploadRequest_ConstructorDefault) {
    auto request = CompleteMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getCompleteAll());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_FALSE(request.hasCompleteMultipartUpload());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, CompleteMultipartUploadRequest_Setter) {
    auto request = CompleteMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getForbidOverwrite());
    EXPECT_EQ("", request.getCompleteAll());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_FALSE(request.hasCompleteMultipartUpload());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setForbidOverwrite("false");
    request.setCompleteAll("yes");
    request.setUploadId("test-upload-id");
    request.setEncodingType("url");

    // Set CompleteMultipartUpload body
    CompleteMultipartUpload completeUpload;
    Part part1;
    part1.setETag("etag1").setPartNumber(1).setSize(1024).setLastModified("2023-01-01T00:00:00Z");
    std::vector<Part> parts = {part1};
    completeUpload.setParts(parts);

    request.setCompleteMultipartUpload(completeUpload);

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("false", request.getForbidOverwrite());
    EXPECT_EQ("yes", request.getCompleteAll());
    EXPECT_EQ("test-upload-id", request.getUploadId());
    EXPECT_EQ("url", request.getEncodingType());
    EXPECT_TRUE(request.hasCompleteMultipartUpload());
    EXPECT_EQ(1, request.getCompleteMultipartUpload().parts.size());
    EXPECT_EQ("etag1", request.getCompleteMultipartUpload().parts[0].eTag);
}

// Test CompleteMultipartUploadResult
TEST(ObjectMultipartTest, CompleteMultipartUploadResult_ConstructorDefault) {
    auto result = CompleteMultipartUploadResult();
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getLocation());
    EXPECT_EQ("", result.getBucket());
    EXPECT_EQ("", result.getKey());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, CompleteMultipartUploadResult_Setters) {
    auto result = CompleteMultipartUploadResult();
    EXPECT_EQ("", result.getVersionId());
    EXPECT_EQ("", result.getLocation());
    EXPECT_EQ("", result.getBucket());
    EXPECT_EQ("", result.getKey());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getEncodingType());

    // Test setters
    result.setLocation("http://example.com/location");
    result.setBucket("test-bucket");
    result.setKey("test-key");
    result.setETag("test-etag");
    result.setEncodingType("url");

    EXPECT_EQ("http://example.com/location", result.getLocation());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("test-etag", result.getETag());
    EXPECT_EQ("url", result.getEncodingType());
}

// Test UploadPartCopyRequest
TEST(ObjectMultipartTest, UploadPartCopyRequest_ConstructorDefault) {
    auto request = UploadPartCopyRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getCopySource());
    EXPECT_EQ("", request.getSourceBucket());
    EXPECT_EQ("", request.getSourceKey());
    EXPECT_EQ("", request.getSourceVersionId());
    EXPECT_EQ("", request.getCopySourceRange());
    EXPECT_EQ("", request.getCopySourceIfMatch());
    EXPECT_EQ("", request.getCopySourceIfNoneMatch());
    EXPECT_EQ("", request.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("", request.getCopySourceIfModifiedSince());
    EXPECT_EQ(-1, request.getPartNumber());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, UploadPartCopyRequest_Setter) {
    auto request = UploadPartCopyRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getCopySource());
    EXPECT_EQ("", request.getSourceBucket());
    EXPECT_EQ("", request.getSourceKey());
    EXPECT_EQ("", request.getSourceVersionId());
    EXPECT_EQ("", request.getCopySourceRange());
    EXPECT_EQ("", request.getCopySourceIfMatch());
    EXPECT_EQ("", request.getCopySourceIfNoneMatch());
    EXPECT_EQ("", request.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("", request.getCopySourceIfModifiedSince());
    EXPECT_EQ(-1, request.getPartNumber());
    EXPECT_EQ("", request.getUploadId());

    // Setter
    request.setBucket("dest-bucket");
    request.setKey("dest-key");
    request.setCopySource("/src-bucket/src-key");
    request.setSourceBucket("src-bucket");
    request.setSourceKey("src-key");
    request.setSourceVersionId("version-id");
    request.setCopySourceRange("bytes=0-1023");
    request.setCopySourceIfMatch("etag123");
    request.setCopySourceIfNoneMatch("etag456");
    request.setCopySourceIfUnmodifiedSince("Wed, 21 Oct 2023 07:28:00 GMT");
    request.setCopySourceIfModifiedSince("Wed, 21 Oct 2023 07:28:00 GMT");
    request.setPartNumber(1);
    request.setUploadId("upload-id");

    EXPECT_EQ("dest-bucket", request.getBucket());
    EXPECT_EQ("dest-key", request.getKey());
    EXPECT_EQ("/src-bucket/src-key", request.getCopySource());
    EXPECT_EQ("src-bucket", request.getSourceBucket());
    EXPECT_EQ("src-key", request.getSourceKey());
    EXPECT_EQ("version-id", request.getSourceVersionId());
    EXPECT_EQ("bytes=0-1023", request.getCopySourceRange());
    EXPECT_EQ("etag123", request.getCopySourceIfMatch());
    EXPECT_EQ("etag456", request.getCopySourceIfNoneMatch());
    EXPECT_EQ("Wed, 21 Oct 2023 07:28:00 GMT", request.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("Wed, 21 Oct 2023 07:28:00 GMT", request.getCopySourceIfModifiedSince());
    EXPECT_EQ(1, request.getPartNumber());
    EXPECT_EQ("upload-id", request.getUploadId());
}

// Test UploadPartCopyResult
TEST(ObjectMultipartTest, UploadPartCopyResult_ConstructorDefault) {
    auto result = UploadPartCopyResult();
    EXPECT_EQ("", result.getCopySourceVersionId());
    EXPECT_EQ("", result.getETag());
    EXPECT_EQ("", result.getLastModified());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, UploadPartCopyResult_ConstructorAll) {
    CopyPartResult body;
    body.eTag = "test-etag";
    body.lastModified = "2023-01-01T00:00:00Z";

    auto result = UploadPartCopyResult(200, {{"x-oss-request-id", "id-123"}}, body);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());
    EXPECT_EQ("test-etag", result.getETag());
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getLastModified());
}

// Test AbortMultipartUploadRequest
TEST(ObjectMultipartTest, AbortMultipartUploadRequest_ConstructorDefault) {
    auto request = AbortMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, AbortMultipartUploadRequest_Setter) {
    auto request = AbortMultipartUploadRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getUploadId());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("test-upload-id", request.getUploadId());
}

// Test AbortMultipartUploadResult
TEST(ObjectMultipartTest, AbortMultipartUploadResult_ConstructorDefault) {
    auto result = AbortMultipartUploadResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, AbortMultipartUploadResult_ConstructorAll) {
    auto result = AbortMultipartUploadResult(204, {{"x-oss-request-id", "id-123"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(204, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());
}

// Test ListMultipartUploadsRequest
TEST(ObjectMultipartTest, ListMultipartUploadsRequest_ConstructorDefault) {
    auto request = ListMultipartUploadsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ(-1, request.getMaxUploads());
    EXPECT_EQ("", request.getKeyMarker());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getUploadIdMarker());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, ListMultipartUploadsRequest_Setter) {
    auto request = ListMultipartUploadsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getDelimiter());
    EXPECT_EQ(-1, request.getMaxUploads());
    EXPECT_EQ("", request.getKeyMarker());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getUploadIdMarker());
    EXPECT_EQ("", request.getEncodingType());

    // Setter
    request.setBucket("test-bucket");
    request.setDelimiter("/");
    request.setMaxUploads(100);
    request.setKeyMarker("key-marker");
    request.setPrefix("prefix/");
    request.setUploadIdMarker("upload-id-marker");
    request.setEncodingType("url");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("/", request.getDelimiter());
    EXPECT_EQ(100, request.getMaxUploads());
    EXPECT_EQ("key-marker", request.getKeyMarker());
    EXPECT_EQ("prefix/", request.getPrefix());
    EXPECT_EQ("upload-id-marker", request.getUploadIdMarker());
    EXPECT_EQ("url", request.getEncodingType());
}

// Test ListMultipartUploadsResult
TEST(ObjectMultipartTest, ListMultipartUploadsResult_ConstructorDefault) {
    auto result = ListMultipartUploadsResult();
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getDelimiter());
    EXPECT_EQ(0, result.getUploads().size());
    EXPECT_EQ("", result.getKeyMarker());
    EXPECT_EQ("", result.getUploadIdMarker());
    EXPECT_EQ("", result.getNextUploadIdMarker());
    EXPECT_EQ(0, result.getCommonPrefixes().size());
    EXPECT_EQ("", result.getBucket());
    EXPECT_EQ("", result.getEncodingType());
    EXPECT_EQ("", result.getNextKeyMarker());
    EXPECT_EQ(-1, result.getMaxUploads());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, ListMultipartUploadsResult_ConstructorAll) {
    ListMultipartUploadsResultXml xmlBody;
    xmlBody.bucket = "test-bucket";
    xmlBody.prefix = "prefix/";
    xmlBody.delimiter = "/";
    xmlBody.keyMarker = "key-marker";
    xmlBody.uploadIdMarker = "upload-id-marker";
    xmlBody.nextUploadIdMarker = "next-upload-id-marker";
    xmlBody.encodingType = "url";
    xmlBody.nextKeyMarker = "next-key-marker";
    xmlBody.maxUploads = 100;
    xmlBody.isTruncated = true;

    // Add some uploads
    Upload upload1;
    upload1.setKey("obj1").setUploadId("upload1").setInitiated("2023-01-01T00:00:00Z");
    xmlBody.uploads.push_back(upload1);

    auto result = ListMultipartUploadsResult(200, {{"x-oss-request-id", "id-123"}}, xmlBody);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("prefix/", result.getPrefix());
    EXPECT_EQ("/", result.getDelimiter());
    EXPECT_EQ(1, result.getUploads().size());
    EXPECT_EQ("key-marker", result.getKeyMarker());
    EXPECT_EQ("upload-id-marker", result.getUploadIdMarker());
    EXPECT_EQ("next-upload-id-marker", result.getNextUploadIdMarker());
    EXPECT_EQ("url", result.getEncodingType());
    EXPECT_EQ("next-key-marker", result.getNextKeyMarker());
    EXPECT_EQ(100, result.getMaxUploads());
    EXPECT_EQ(true, result.getIsTruncated());
    EXPECT_EQ("obj1", result.getUploads()[0].key);
    EXPECT_EQ("upload1", result.getUploads()[0].uploadId);
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getUploads()[0].initiated);
}

// Test ListPartsRequest
TEST(ObjectMultipartTest, ListPartsRequest_ConstructorDefault) {
    auto request = ListPartsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ(-1, request.getMaxParts());
    EXPECT_EQ(-1, request.getPartNumberMarker());
    EXPECT_EQ("", request.getEncodingType());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectMultipartTest, ListPartsRequest_Setter) {
    auto request = ListPartsRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getUploadId());
    EXPECT_EQ(-1, request.getMaxParts());
    EXPECT_EQ(-1, request.getPartNumberMarker());
    EXPECT_EQ("", request.getEncodingType());

    // Setter
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("test-upload-id");
    request.setMaxParts(100);
    request.setPartNumberMarker(5);
    request.setEncodingType("url");

    EXPECT_EQ("test-bucket", request.getBucket());
    EXPECT_EQ("test-key", request.getKey());
    EXPECT_EQ("test-upload-id", request.getUploadId());
    EXPECT_EQ(100, request.getMaxParts());
    EXPECT_EQ(5, request.getPartNumberMarker());
    EXPECT_EQ("url", request.getEncodingType());
}

// Test ListPartsResult
TEST(ObjectMultipartTest, ListPartsResult_ConstructorDefault) {
    auto result = ListPartsResult();
    EXPECT_EQ("", result.getBucket());
    EXPECT_EQ("", result.getKey());
    EXPECT_EQ("", result.getUploadId());
    EXPECT_EQ(-1, result.getPartNumberMarker());
    EXPECT_EQ(-1, result.getNextPartNumberMarker());
    EXPECT_EQ(-1, result.getMaxParts());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(0, result.getParts().size());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectMultipartTest, ListPartsResult_ConstructorAll) {
    ListPartResultXml xmlBody;
    xmlBody.bucket = "test-bucket";
    xmlBody.key = "test-key";
    xmlBody.uploadId = "test-upload-id";
    xmlBody.partNumberMarker = 5;
    xmlBody.nextPartNumberMarker = 10;
    xmlBody.maxParts = 100;
    xmlBody.isTruncated = true;

    // Add some parts
    Part part1;
    part1.setETag("etag1").setPartNumber(1).setSize(1024).setLastModified("2023-01-01T00:00:00Z");
    xmlBody.parts.push_back(part1);

    auto result = ListPartsResult(200, {{"x-oss-request-id", "id-123"}}, xmlBody);
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(1, result.getHeaders().size());
    EXPECT_EQ("test-bucket", result.getBucket());
    EXPECT_EQ("test-key", result.getKey());
    EXPECT_EQ("test-upload-id", result.getUploadId());
    EXPECT_EQ(5, result.getPartNumberMarker());
    EXPECT_EQ(10, result.getNextPartNumberMarker());
    EXPECT_EQ(100, result.getMaxParts());
    EXPECT_EQ(true, result.getIsTruncated());
    EXPECT_EQ(1, result.getParts().size());
    EXPECT_EQ("etag1", result.getParts()[0].eTag);
    EXPECT_EQ(1, result.getParts()[0].partNumber);
    EXPECT_EQ(1024, result.getParts()[0].size);
    EXPECT_EQ("2023-01-01T00:00:00Z", result.getParts()[0].lastModified);
}

} // namespace models
} // namespace oss2
} // namespace alibabacloud