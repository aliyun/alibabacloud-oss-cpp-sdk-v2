#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/Paginator.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class PaginatorTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());

        for (int i = 0; i < 5; i++) {
            auto key = "paginator-test-obj-" + std::to_string(i);
            auto putOutcome = client->putObject(
                    models::PutObjectRequest()
                            .setBucket(bucketName_)
                            .setKey(key)
                            .setBody(RequestBody::FromString("data-" + std::to_string(i))));
            EXPECT_TRUE(putOutcome.has_value());
        }
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string PaginatorTest::bucketName_ = "";

TEST_F(PaginatorTest, ListObjectsV2_Paginate) {
    auto client = ClientHelper::GetDefaultClient();

    auto request = models::ListObjectsV2Request()
                           .setBucket(bucketName_)
                           .setMaxKeys(2);

    auto paginator = makePaginator(*client, request);

    int pageCount = 0;
    std::vector<std::string> allKeys;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        ASSERT_TRUE(outcome.has_value());
        for (auto& obj : outcome.value().getContents()) {
            allKeys.push_back(obj.key);
        }
        pageCount++;
    }

    EXPECT_GE(pageCount, 3);
    ASSERT_EQ(5u, allKeys.size());
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ("paginator-test-obj-" + std::to_string(i), allKeys[i]);
    }
}

TEST_F(PaginatorTest, ListObjects_Paginate) {
    auto client = ClientHelper::GetDefaultClient();

    auto request = models::ListObjectsRequest()
                           .setBucket(bucketName_)
                           .setMaxKeys(2);

    auto paginator = makePaginator(*client, request);

    int pageCount = 0;
    std::vector<std::string> allKeys;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        ASSERT_TRUE(outcome.has_value());
        for (auto& obj : outcome.value().getContents()) {
            allKeys.push_back(obj.key);
        }
        pageCount++;
    }

    EXPECT_GE(pageCount, 3);
    ASSERT_EQ(5u, allKeys.size());
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ("paginator-test-obj-" + std::to_string(i), allKeys[i]);
    }
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
