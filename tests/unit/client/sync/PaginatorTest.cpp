#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/Paginator.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

static OSSClient makeClient(std::shared_ptr<MockTransport> mock) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    return OSSClient(config);
}

TEST(PaginatorTest, ListObjectsV2_SinglePage) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>false</IsTruncated>
    <Contents><Key>obj1</Key></Contents>
</ListBucketResult>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>(body)}));

    auto request = models::ListObjectsV2Request().setBucket("test-bucket");
    auto paginator = makePaginator(client, request);

    EXPECT_TRUE(paginator.hasNext());
    auto outcome = paginator.nextPage();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1u, outcome.value().getContents().size());
    EXPECT_FALSE(paginator.hasNext());
}

TEST(PaginatorTest, ListObjectsV2_MultiPage) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    auto body1 = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>true</IsTruncated>
    <NextContinuationToken>token-abc</NextContinuationToken>
    <Contents><Key>obj1</Key></Contents>
    <Contents><Key>obj2</Key></Contents>
</ListBucketResult>)";

    auto body2 = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>false</IsTruncated>
    <Contents><Key>obj3</Key></Contents>
</ListBucketResult>)";

    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>(body1)}));
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>(body2)}));

    auto request = models::ListObjectsV2Request().setBucket("test-bucket");
    auto paginator = makePaginator(client, request);

    std::vector<std::string> allKeys;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        ASSERT_TRUE(outcome.has_value());
        for (auto& obj : outcome.value().getContents()) {
            allKeys.push_back(obj.key);
        }
    }

    EXPECT_EQ(3u, allKeys.size());
    EXPECT_EQ("obj1", allKeys[0]);
    EXPECT_EQ("obj2", allKeys[1]);
    EXPECT_EQ("obj3", allKeys[2]);
}

TEST(PaginatorTest, ListObjects_MultiPage) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    auto body1 = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>true</IsTruncated>
    <NextMarker>obj2</NextMarker>
    <Contents><Key>obj1</Key></Contents>
    <Contents><Key>obj2</Key></Contents>
</ListBucketResult>)";

    auto body2 = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>false</IsTruncated>
    <Contents><Key>obj3</Key></Contents>
</ListBucketResult>)";

    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>(body1)}));
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-2"}}, std::make_shared<std::stringstream>(body2)}));

    auto request = models::ListObjectsRequest().setBucket("test-bucket");
    auto paginator = makePaginator(client, request);

    int pageCount = 0;
    std::size_t totalObjects = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        ASSERT_TRUE(outcome.has_value());
        totalObjects += outcome.value().getContents().size();
        pageCount++;
    }

    EXPECT_EQ(2, pageCount);
    EXPECT_EQ(3u, totalObjects);
}

TEST(PaginatorTest, MakePaginator_WithPointer) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    auto body = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult>
    <Name>test-bucket</Name>
    <IsTruncated>false</IsTruncated>
    <Contents><Key>obj1</Key></Contents>
</ListBucketResult>)";
    mock->responses.emplace_back(std::make_unique<ResponseMessage>(
            ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1"}}, std::make_shared<std::stringstream>(body)}));

    OSSClient* ptr = &client;
    auto request = models::ListObjectsV2Request().setBucket("test-bucket");
    auto paginator = makePaginator(ptr, request);

    EXPECT_TRUE(paginator.hasNext());
    auto outcome = paginator.nextPage();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(paginator.hasNext());
}

TEST(PaginatorTest, ErrorStopsPagination) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    // No responses - transport error
    auto request = models::ListObjectsV2Request().setBucket("test-bucket");
    auto paginator = makePaginator(client, request);

    EXPECT_TRUE(paginator.hasNext());
    auto outcome = paginator.nextPage();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_FALSE(paginator.hasNext());
}

} // namespace alibabacloud::oss2
