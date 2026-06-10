#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {


TEST(OSSClientRegionTest, DescribeRegions_FullXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<RegionInfoList>
  <RegionInfo>
    <Region>oss-cn-hangzhou</Region>
    <InternetEndpoint>oss-cn-hangzhou.aliyuncs.com</InternetEndpoint>
    <InternalEndpoint>oss-cn-hangzhou-internal.aliyuncs.com</InternalEndpoint>
    <AccelerateEndpoint>oss-accelerate.aliyuncs.com</AccelerateEndpoint>
  </RegionInfo>
  <RegionInfo>
    <Region>oss-cn-shenzhen</Region>
    <InternetEndpoint>oss-cn-shenzhen.aliyuncs.com</InternetEndpoint>
    <InternalEndpoint>oss-cn-shenzhen-internal.aliyuncs.com</InternalEndpoint>
    <AccelerateEndpoint>oss-accelerate.aliyuncs.com</AccelerateEndpoint>
  </RegionInfo>
</RegionInfoList>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DescribeRegionsRequest();
    auto outcome = client.describeRegions(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasRegionInfoList());
    EXPECT_EQ(2, result.getRegionInfoList().regionInfos.size());

    EXPECT_EQ("oss-cn-hangzhou", result.getRegionInfoList().regionInfos.at(0).region);
    EXPECT_EQ("oss-cn-hangzhou.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internetEndpoint);
    EXPECT_EQ("oss-cn-hangzhou-internal.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", result.getRegionInfoList().regionInfos.at(0).accelerateEndpoint);


    EXPECT_EQ("oss-cn-shenzhen", result.getRegionInfoList().regionInfos.at(1).region);
    EXPECT_EQ("oss-cn-shenzhen.aliyuncs.com", result.getRegionInfoList().regionInfos.at(1).internetEndpoint);
    EXPECT_EQ("oss-cn-shenzhen-internal.aliyuncs.com", result.getRegionInfoList().regionInfos.at(1).internalEndpoint);
    EXPECT_EQ("oss-accelerate.aliyuncs.com", result.getRegionInfoList().regionInfos.at(1).accelerateEndpoint);
}


TEST(OSSClientRegionTest, DescribeRegions_EmptyXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<RegionInfoList>
</RegionInfoList>
    )";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DescribeRegionsRequest();
    auto outcome = client.describeRegions(request);
    EXPECT_TRUE(outcome.has_value());
    auto& result = outcome.value();

    EXPECT_EQ(200, result.getStatusCode());
    EXPECT_EQ("id-1234", result.getRequestId());

    EXPECT_EQ(true, result.hasRegionInfoList());
    EXPECT_EQ(0, result.getRegionInfoList().regionInfos.size());
}


TEST(OSSClientRegionTest, DescribeRegions_ErrorXml) {
    auto mockHandler = std::make_shared<MockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mockHandler;

    auto client = OSSClient(config);

    auto body = R"(ERROR)";

    mockHandler->Clear();
    mockHandler->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::DescribeRegionsRequest();
    auto outcome = client.describeRegions(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

#ifdef ALIBABACLOUD_OSS_HAS_TINYXML2
    EXPECT_EQ("XMLError:8", error.getCode());
    EXPECT_EQ("Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1", error.getMessage());
#else
    EXPECT_EQ("XMLError:10", error.getCode());
    EXPECT_EQ("Error=XML_ERROR_PARSING_TEXT ErrorID=10 (0xa) Line number=1", error.getMessage());
#endif
    EXPECT_EQ("ERROR", error.getSnapshot());
    EXPECT_EQ("id-1234", error.getRequestId());
}


TEST(OSSClientRegionTest, DescribeRegions_ErrorResponse) {
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

    auto request = models::DescribeRegionsRequest();
    auto outcome = client.describeRegions(request);
    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();

    EXPECT_EQ(403, error.getStatusCode());
    EXPECT_EQ("DescribeRegions", error.getOpName());
    EXPECT_EQ("GET", error.getMethod());
    EXPECT_EQ("https://oss-cn-hangzhou.aliyuncs.com/?regions", error.getRequestTarget());
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

} // namespace alibabacloud::oss2