#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/Service.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(ServiceTest, ListBucketsRequest_ConstructorDefault) {
    // Default
    auto request = ListBucketsRequest();
    EXPECT_EQ("", request.getMarker());
    EXPECT_EQ("", request.getResourceGroupId());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ("", request.getTagKey());
    EXPECT_EQ("", request.getTagValue());
    EXPECT_EQ(-1, request.getMaxKeys());
}

TEST(ServiceTest, ListBucketsRequest_Setter) {
    auto request = ListBucketsRequest();
    EXPECT_EQ("", request.getMarker());
    EXPECT_EQ("", request.getResourceGroupId());
    EXPECT_EQ("", request.getPrefix());
    EXPECT_EQ("", request.getTagging());
    EXPECT_EQ("", request.getTagKey());
    EXPECT_EQ("", request.getTagValue());
    EXPECT_EQ(-1, request.getMaxKeys());

    // Setter
    request.setMarker("bucket-123");
    request.setResourceGroupId("rg-123");
    request.setPrefix("prefix-123");
    request.setTagging("tagging-123");
    request.setTagKey("key-123");
    request.setTagValue("vakye-123");
    request.setMaxKeys(10);

    EXPECT_EQ("bucket-123", request.getMarker());
    EXPECT_EQ("rg-123", request.getResourceGroupId());
    EXPECT_EQ("prefix-123", request.getPrefix());
    EXPECT_EQ("tagging-123", request.getTagging());
    EXPECT_EQ("key-123", request.getTagKey());
    EXPECT_EQ("vakye-123", request.getTagValue());
    EXPECT_EQ(10, request.getMaxKeys());
}

TEST(ServiceTest, ListBucketsResult_ConstructorDefault) {
    auto result = ListBucketsResult();
    EXPECT_EQ("", result.getMarker());
    EXPECT_EQ("", result.getNextMarker());
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getOwner().id.value_or(""));
    EXPECT_EQ("", result.getOwner().displayName.value_or(""));
    EXPECT_EQ(-1, result.getMaxKeys());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(true, result.getBuckets().empty());

    EXPECT_EQ(0, result.getStatusCode());
    EXPECT_EQ("", result.getRequestId());
    EXPECT_EQ(true, result.getHeaders().empty());
}

TEST(ServiceTest, ListBucketsResult_ConstructorPart) {
    auto result = ListBucketsResult(200, {{"x-oss-request-id", "id-123"}, {"Content-Type", "application/xml"}});
    EXPECT_EQ("", result.getMarker());
    EXPECT_EQ("", result.getNextMarker());
    EXPECT_EQ("", result.getPrefix());
    EXPECT_EQ("", result.getOwner().id.value_or(""));
    EXPECT_EQ("", result.getOwner().displayName.value_or(""));
    EXPECT_EQ(-1, result.getMaxKeys());
    EXPECT_EQ(false, result.getIsTruncated());
    EXPECT_EQ(true, result.getBuckets().empty());
    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-123", result.getRequestId());
    EXPECT_EQ(2, result.getHeaders().size());
    EXPECT_EQ("id-123", result.getHeaders().at("x-oss-request-id"));
    EXPECT_EQ("application/xml", result.getHeaders().at("content-type"));
    EXPECT_EQ("application/xml", result.getHeaders().at("Content-Type"));
}

TEST(ServiceTest, ListBucketsResult_ConstructorAll) {
    auto body = ListAllMyBucketsResult{
            10,
            true,
            "nextMarker",
            {{"cn-hangzhou", "2023-11-09T08:59:08.000Z", "oss-cn-hangzhou.aliyuncs.com",
              "oss-cn-hangzhou-internal.aliyuncs.com", "oss-cn-hangzhou", "bucket-1", "Standard"},
             {"cn-shenzhen", "2024-11-09T08:59:08.000Z", "oss-cn-shenzhen.aliyuncs.com",
              "oss-cn-shenzhen-internal.aliyuncs.com", "oss-cn-shenzhen", "bucket-2", "IA"}},
            {"id", "display"},
            "prefix",
            "marker"};
    auto result = ListBucketsResult(200, {{"x-oss-request-id", "id-1234"}, {"Content-Type", "application/xml"}}, body);
    EXPECT_EQ("marker", result.getMarker());
    EXPECT_EQ("nextMarker", result.getNextMarker());
    EXPECT_EQ("prefix", result.getPrefix());
    EXPECT_EQ("id", result.getOwner().id.value_or(""));
    EXPECT_EQ("display", result.getOwner().displayName.value_or(""));
    EXPECT_EQ(10, result.getMaxKeys());
    EXPECT_EQ(true, result.getIsTruncated());
    EXPECT_EQ(2, result.getBuckets().size());

    EXPECT_EQ("cn-hangzhou", result.getBuckets().at(0).region);
    EXPECT_EQ("2023-11-09T08:59:08.000Z", result.getBuckets().at(0).creationDate);
    EXPECT_EQ("oss-cn-hangzhou-internal.aliyuncs.com", result.getBuckets().at(0).intranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", result.getBuckets().at(0).extranetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou", result.getBuckets().at(0).location);
    EXPECT_EQ("bucket-1", result.getBuckets().at(0).name);
    EXPECT_EQ("Standard", result.getBuckets().at(0).storageClass);

    EXPECT_EQ("cn-shenzhen", result.getBuckets().at(1).region);
    EXPECT_EQ("2024-11-09T08:59:08.000Z", result.getBuckets().at(1).creationDate);
    EXPECT_EQ("oss-cn-shenzhen-internal.aliyuncs.com", result.getBuckets().at(1).intranetEndpoint);
    EXPECT_EQ("oss-cn-shenzhen.aliyuncs.com", result.getBuckets().at(1).extranetEndpoint);
    EXPECT_EQ("oss-cn-shenzhen", result.getBuckets().at(1).location);
    EXPECT_EQ("bucket-2", result.getBuckets().at(1).name);
    EXPECT_EQ("IA", result.getBuckets().at(1).storageClass);


    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());
    EXPECT_EQ(2, result.getHeaders().size());
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud