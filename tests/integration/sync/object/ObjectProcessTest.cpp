#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"

#include <fstream>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObjectProcessTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());

        std::string filePath = TEST_DATA_PATH "example.jpg";
        std::ifstream ifs(filePath, std::ios::binary);
        ASSERT_TRUE(ifs.good());
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        auto putOutcome = client->putObject(
            models::PutObjectRequest()
                .setBucket(bucketName_)
                .setKey(imageKey_)
                .setBody(RequestBody::fromString(content)));
        EXPECT_TRUE(putOutcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
    static std::string imageKey_;
};

std::string ObjectProcessTest::bucketName_ = "";
std::string ObjectProcessTest::imageKey_ = "test-process-source.jpg";

TEST_F(ObjectProcessTest, ProcessObject_ImageResize) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-process-target.jpg";
    std::string saveas = utils::Base64Encode(bucketName_) + "," + utils::Base64Encode(targetKey);
    std::string process = "image/resize,w_100|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto outcome = client->processObject(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process));
    EXPECT_TRUE(outcome.has_value());

    const auto& body = outcome.value().getBody();
    EXPECT_FALSE(body.empty());
    EXPECT_NE(body.find("\"bucket\""), std::string::npos);
    EXPECT_NE(body.find("\"object\""), std::string::npos);
    EXPECT_NE(body.find("\"status\""), std::string::npos);
    EXPECT_NE(body.find(targetKey), std::string::npos);

    auto headOutcome = client->headObject(
        models::HeadObjectRequest()
            .setBucket(bucketName_)
            .setKey(targetKey));
    EXPECT_TRUE(headOutcome.has_value());
}

TEST_F(ObjectProcessTest, ProcessObject_InvalidProcess) {
    auto client = ClientHelper::GetDefaultClient();

    auto outcome = client->processObject(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess("invalid/process"));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(ObjectProcessTest, ProcessObject_KeyNotExist) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-process-target-noexist.jpg";
    std::string process = "image/resize,w_100|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto outcome = client->processObject(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey("no-such-key-process-test.jpg")
            .setProcess(process));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(ObjectProcessTest, ProcessObject_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    std::string process = "image/resize,w_100";
    auto outcome = client->processObject(
        models::ProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process));
    EXPECT_FALSE(outcome.has_value());
}

TEST_F(ObjectProcessTest, AsyncProcessObject_Normal) {
    auto client = ClientHelper::GetDefaultClient();

    std::string targetKey = "test-async-process-target.mp4";
    std::string process = "video/convert,f_mp4|sys/saveas,o_" + utils::Base64Encode(targetKey) + ",b_" + utils::Base64Encode(bucketName_);

    auto outcome = client->asyncProcessObject(
        models::AsyncProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process));

    if (outcome.has_value()) {
        const auto& body = outcome.value().getBody();
        EXPECT_FALSE(body.empty());
        EXPECT_NE(body.find("\"EventId\""), std::string::npos);
        EXPECT_NE(body.find("\"TaskId\""), std::string::npos);
        EXPECT_NE(body.find("\"RequestId\""), std::string::npos);
    }
}

TEST_F(ObjectProcessTest, AsyncProcessObject_InvalidCredentials) {
    auto client = ClientHelper::GetInvalidClient();

    std::string process = "video/convert,f_mp4";
    auto outcome = client->asyncProcessObject(
        models::AsyncProcessObjectRequest()
            .setBucket(bucketName_)
            .setKey(imageKey_)
            .setProcess(process));
    EXPECT_FALSE(outcome.has_value());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
