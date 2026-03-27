#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/BucketBasic.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(PutBucketRequestTest, EmptyConstructor) {
    auto request = PutBucketRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getAcl());
    EXPECT_EQ("", request.getResourceGroupId());
    EXPECT_EQ("", request.getBucketTagging());
}

TEST(PutBucketRequestTest, FullConstructor) {
    auto headers = HeaderCollection();
    headers.emplace("x-oss-acl", "private");
    headers.emplace("x-oss-bucket-tagging", "tagging");
    headers.emplace("x-oss-resource-group-id", "rg-id");
    /*
        auto request = PutBucketRequest("bucket", std::move(headers), ParameterCollection());
        EXPECT_EQ("bucket", request.getBucket());
        EXPECT_EQ("private", request.getAcl());
        EXPECT_EQ("tagging", request.getBucketTagging());
        EXPECT_EQ("rg-id", request.getResourceGroupId());

        request = PutBucketRequest("bucket");
        EXPECT_EQ("bucket", request.getBucket());
        EXPECT_EQ("", request.getAcl());
        EXPECT_EQ("", request.getResourceGroupId());
        EXPECT_EQ("", request.getBucketTagging());
    */
}

TEST(PutBucketRequestTest, HeaderSetter) {
    auto request = PutBucketRequest();
    EXPECT_EQ("", request.getBucket());
    EXPECT_EQ("", request.getAcl());
    EXPECT_EQ("", request.getBucketTagging());
    EXPECT_EQ("", request.getResourceGroupId());

    request.setAcl("private").setBucket("bucket").setBucketTagging("tagging").setResourceGroupId("rg-id");
    EXPECT_EQ("bucket", request.getBucket());
    EXPECT_EQ("private", request.getAcl());
    EXPECT_EQ("tagging", request.getBucketTagging());
    EXPECT_EQ("rg-id", request.getResourceGroupId());

    request.setAcl("public").setBucket("bucket1").setBucketTagging("tagging1").setResourceGroupId("rg-id1");
    EXPECT_EQ("bucket1", request.getBucket());
    EXPECT_EQ("public", request.getAcl());
    EXPECT_EQ("tagging1", request.getBucketTagging());
    EXPECT_EQ("rg-id1", request.getResourceGroupId());
}

TEST(PutBucketRequestTest, BodySetter) {
    auto request = PutBucketRequest();
    EXPECT_FALSE(request.hasCreateBucketConfiguration());

    request.setCreateBucketConfiguration({"IA", "ZRS"});
    EXPECT_TRUE(request.hasCreateBucketConfiguration());
    auto& value = request.getCreateBucketConfiguration();
    EXPECT_EQ("IA", value.storageClass);
    EXPECT_EQ("ZRS", value.dataRedundancyType);

    request.setCreateBucketConfiguration({"Archive", "LRS"});
    auto& value2 = request.getCreateBucketConfiguration();
    EXPECT_EQ("Archive", value2.storageClass);
    EXPECT_EQ("LRS", value2.dataRedundancyType);
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud
