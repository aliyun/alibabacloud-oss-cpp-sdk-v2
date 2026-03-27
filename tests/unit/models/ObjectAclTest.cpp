#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/ObjectAcl.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(ObjectAclTest, GetObjectAclRequest_ConstructorDefault) {
    // Default
    auto request = GetObjectAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectAclTest, GetBucketAclRequest_Setter) {
    auto request = GetObjectAclRequest();
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

TEST(ObjectAclTest, GetObjectAclResult_ConstructorDefault) {
    auto result = GetObjectAclResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasAccessControlPolicy());
    EXPECT_EQ(false, result.getAccessControlPolicy().accessControlList.has_value());
    EXPECT_EQ(false, result.getAccessControlPolicy().owner.has_value());
}

TEST(ObjectAclTest, GetObjectAclResult_ConstructorAll) {
    auto result = GetObjectAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasAccessControlPolicy());
    EXPECT_EQ(false, result.getAccessControlPolicy().accessControlList.has_value());
    EXPECT_EQ(false, result.getAccessControlPolicy().owner.has_value());
}

TEST(ObjectAclTest, GetObjectAclResult_SetBody) {
    auto result = GetObjectAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    result.setAccessControlPolicy({Owner{"id-123", "id-desc"}, AccessControlList{"private"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasAccessControlPolicy());
    EXPECT_EQ("id-123", result.getAccessControlPolicy().owner.value().id);
    EXPECT_EQ("id-desc", result.getAccessControlPolicy().owner.value().displayName);
    EXPECT_EQ("private", result.getAccessControlPolicy().accessControlList.value().grant);
}


TEST(ObjectAclTest, PutObjectAclRequest_ConstructorDefault) {
    // Default
    auto request = PutObjectAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectAclTest, PutObjectAclRequest_Setter) {
    auto request = PutObjectAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setKey("key");
    request.setVersionId("id");
    request.setObjectAcl("private");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("key", request.getKey());
    EXPECT_EQ("id", request.getVersionId());
    EXPECT_EQ("private", request.getObjectAcl());
}

TEST(ObjectAclTest, PutObjectAclResult_ConstructorDefault) {
    auto result = PutObjectAclResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(ObjectAclTest, PutObjectAclResult_ConstructorAll) {
    auto result = PutObjectAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud