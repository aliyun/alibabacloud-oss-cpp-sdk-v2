#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/ObjectTagging.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(ObjectTaggingTest, GetObjectTaggingRequest_ConstructorDefault) {
    // Default
    auto request = GetObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectTaggingTest, GetBucketAclRequest_Setter) {
    auto request = GetObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setKey("key");
    request.setVersionId("version-id");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("key", request.getKey());
    EXPECT_EQ("version-id", request.getVersionId());
}

TEST(ObjectTaggingTest, GetObjectTaggingResult_ConstructorDefault) {
    auto result = GetObjectTaggingResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasTagging());
    EXPECT_EQ(false, result.getTagging().tagSet.has_value());
}

TEST(ObjectTaggingTest, GetObjectTaggingResult_ConstructorAll) {
    auto result = GetObjectTaggingResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasTagging());
    EXPECT_EQ(false, result.getTagging().tagSet.has_value());
}

TEST(ObjectTaggingTest, GetObjectTaggingResult_SetBody) {
    auto result = GetObjectTaggingResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    result.setTagging({Tagging{TagSet{{Tag{"key1", "value1"}, Tag{"key2", "value2"}}}}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasTagging());
    EXPECT_EQ(true, result.getTagging().tagSet.has_value());
    EXPECT_EQ(2, result.getTagging().tagSet.value().tags.size());
    EXPECT_EQ("key1", result.getTagging().tagSet.value().tags.at(0).key);
    EXPECT_EQ("value1", result.getTagging().tagSet.value().tags.at(0).value);

    EXPECT_EQ("key2", result.getTagging().tagSet.value().tags.at(1).key);
    EXPECT_EQ("value2", result.getTagging().tagSet.value().tags.at(1).value);
}


TEST(ObjectTaggingTest, PutObjectTaggingRequest_ConstructorDefault) {
    // Default
    auto request = PutObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectTaggingTest, PutObjectTaggingRequest_Setter) {
    auto request = PutObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setKey("key");
    request.setVersionId("id");
    request.setTagging({Tagging{TagSet{{Tag{"key1", "value1"}, Tag{"key2", "value2"}}}}});
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("key", request.getKey());
    EXPECT_EQ("id", request.getVersionId());
    EXPECT_EQ(true, request.hasTagging());
    EXPECT_EQ(true, request.getTagging().tagSet.has_value());
    EXPECT_EQ(2, request.getTagging().tagSet.value().tags.size());
    EXPECT_EQ("key1", request.getTagging().tagSet.value().tags.at(0).key);
    EXPECT_EQ("value1", request.getTagging().tagSet.value().tags.at(0).value);

    EXPECT_EQ("key2", request.getTagging().tagSet.value().tags.at(1).key);
    EXPECT_EQ("value2", request.getTagging().tagSet.value().tags.at(1).value);
}

TEST(ObjectTaggingTest, PutObjectTaggingResult_ConstructorDefault) {
    auto result = PutObjectTaggingResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectTaggingTest, PutObjectTaggingResult_ConstructorAll) {
    auto result = PutObjectTaggingResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}


TEST(ObjectTaggingTest, DeleteObjectTaggingRequest_ConstructorDefault) {
    // Default
    auto request = DeleteObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectTaggingTest, DeleteObjectTaggingRequest_Setter) {
    auto request = DeleteObjectTaggingRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setKey("key");
    request.setVersionId("id");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("key", request.getKey());
    EXPECT_EQ("id", request.getVersionId());
}

TEST(ObjectTaggingTest, DeleteObjectTaggingResult_ConstructorDefault) {
    auto result = DeleteObjectTaggingResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectTaggingTest, DeleteObjectTaggingResult_ConstructorAll) {
    auto result = DeleteObjectTaggingResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud