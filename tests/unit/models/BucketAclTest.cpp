#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/BucketAcl.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(BucketAclTest, GetBucketAclRequest_ConstructorDefault) {
    // Default
    auto request = GetBucketAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketAclTest, GetBucketAclRequest_Setter) {
    auto request = GetBucketAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    EXPECT_EQ("bucket", request.getBucket());
}

TEST(BucketAclTest, GetBucketAclResult_ConstructorDefault) {
    auto result = GetBucketAclResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ(false, result.hasAccessControlPolicy());
    EXPECT_EQ(false, result.getAccessControlPolicy().accessControlList.has_value());
    EXPECT_EQ(false, result.getAccessControlPolicy().owner.has_value());
}

TEST(BucketAclTest, GetBucketAclResult_ConstructorAll) {
    auto result = GetBucketAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(false, result.hasAccessControlPolicy());
    EXPECT_EQ(false, result.getAccessControlPolicy().accessControlList.has_value());
    EXPECT_EQ(false, result.getAccessControlPolicy().owner.has_value());
}

TEST(BucketAclTest, GetBucketAclResult_SetBody) {
    auto result = GetBucketAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    result.setAccessControlPolicy({Owner{"id-123", "id-desc"}, AccessControlList{"private"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ(true, result.hasAccessControlPolicy());
    EXPECT_EQ("id-123", result.getAccessControlPolicy().owner.value().id);
    EXPECT_EQ("id-desc", result.getAccessControlPolicy().owner.value().displayName);
    EXPECT_EQ("private", result.getAccessControlPolicy().accessControlList.value().grant);
}


TEST(BucketAclTest, PutBucketAclRequest_ConstructorDefault) {
    // Default
    auto request = PutBucketAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(BucketAclTest, PutBucketAclRequest_Setter) {
    auto request = PutBucketAclRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setAcl("private");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("private", request.getAcl());
}

TEST(BucketAclTest, PutBucketAclResult_ConstructorDefault) {
    auto result = PutBucketAclResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
}

TEST(BucketAclTest, PutBucketAclResult_ConstructorAll) {
    auto result = PutBucketAclResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
}

TEST(BucketAclTest, Owner_ConstructorAll) {
    auto owner = Owner();
    EXPECT_EQ(false, owner.id.has_value());
    EXPECT_EQ(false, owner.displayName.has_value());

    owner.setId("id-123");
    owner.setDisplayName("id-desc");
    EXPECT_EQ("id-123", owner.id);
    EXPECT_EQ("id-desc", owner.displayName);
}

TEST(BucketAclTest, AccessControlList_ConstructorAll) {
    auto acl = AccessControlList();
    EXPECT_EQ(false, acl.grant.has_value());

    acl.setGrant("public");
    EXPECT_EQ("public", acl.grant);
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud