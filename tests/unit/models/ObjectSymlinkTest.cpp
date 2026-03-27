#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/ObjectSymlink.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(ObjectSymlinkTest, GetSymlinkRequest_ConstructorDefault) {
    // Default
    auto request = GetSymlinkRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getKey());
    EXPECT_EQ("", request.getVersionId());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectSymlinkTest, GetBucketAclRequest_Setter) {
    auto request = GetSymlinkRequest();
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

TEST(ObjectSymlinkTest, GetSymlinkResult_ConstructorDefault) {
    auto result = GetSymlinkResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ("", result.getSymlinkTarget());
    EXPECT_EQ("", result.getVersionId());
}

TEST(ObjectSymlinkTest, GetSymlinkResult_ConstructorAll) {
    auto result = GetSymlinkResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ("", result.getSymlinkTarget());
    EXPECT_EQ("", result.getVersionId());
}

TEST(ObjectSymlinkTest, GetSymlinkResult_SetHeaders) {
    auto result = GetSymlinkResult(200, {
                                                {"x-oss-request-id", "id-123"},
                                                {"Content-Type", "application/xml"},
                                                {"x-oss-version-id", "v-id"},
                                                {"x-oss-symlink-target", "source-key"},
                                        });
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(4, result.getHeaders().size());
    EXPECT_EQ("source-key", result.getSymlinkTarget());
    EXPECT_EQ("v-id", result.getVersionId());
}


TEST(ObjectSymlinkTest, PutSymlinkRequest_ConstructorDefault) {
    // Default
    auto request = PutSymlinkRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());
}

TEST(ObjectSymlinkTest, PutSymlinkRequest_Setter) {
    auto request = PutSymlinkRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ(0, request.getHeaders().size());
    EXPECT_EQ(0, request.getParameters().size());

    // Setter
    request.setBucket("bucket");
    request.setKey("key");
    request.setForbidOverwrite("true");
    request.setObjectAcl("private");
    request.setStorageClass("IA");
    request.setSymlinkTarget("source-key");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("key", request.getKey());
    EXPECT_EQ("private", request.getObjectAcl());
    EXPECT_EQ("source-key", request.getSymlinkTarget());
    EXPECT_EQ("IA", request.getStorageClass());
    EXPECT_EQ("true", request.getForbidOverwrite());
}

TEST(ObjectSymlinkTest, PutSymlinkResult_ConstructorDefault) {
    auto result = PutSymlinkResult();
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ(0, result.getHeaders().size());
    EXPECT_EQ("", result.getVersionId());
}

TEST(ObjectSymlinkTest, PutSymlinkResult_ConstructorAll) {
    auto result = PutSymlinkResult(
            200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}, {"x-oss-version-id", "v-id"}});
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ(3, result.getHeaders().size());

    EXPECT_EQ("v-id", result.getVersionId());
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud