#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

class ObservableWriterTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto outcome = client->putBucket(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(outcome.has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string ObservableWriterTest::bucketName_ = "";

TEST_F(ObservableWriterTest, GetObject_WithProgressAndCRC) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "observable-writer-test";
    std::string content(128 * 1024, 'A');

    auto putOutcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto output = std::make_shared<std::stringstream>();
    auto writer = std::make_shared<OStreamWriter>(output);
    auto crc = std::make_shared<CRC64WriteObserver>();

    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;
    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred,
                            std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(content.size()), total);
    };
    auto progressObs = std::make_shared<ProgressWriteObserver>(progress,
            static_cast<std::int64_t>(content.size()));

    auto sink = std::make_shared<ObservableWriter>(writer, progressObs, crc);

    SinkFactory factory;
    factory.supplier = [sink](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> { return sink; };
    factory.isOneShot = false;

    auto outcome = client->getObject(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), output->str().size());
    EXPECT_EQ(content, output->str());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(content.size(), lastTransferred);
    EXPECT_EQ(content.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST_F(ObservableWriterTest, GetObject_RetryWithObserverReset) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "observable-writer-retry-test";
    std::string content = "retry content for observable writer";

    auto putOutcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto crc = std::make_shared<CRC64WriteObserver>();

    std::size_t totalIncrement = 0;
    int callCount = 0;
    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t, std::int64_t, std::uintptr_t) {
        totalIncrement += increment;
        callCount++;
    };
    auto progressObs = std::make_shared<ProgressWriteObserver>(progress,
            static_cast<std::int64_t>(content.size()));

    int supplierCallCount = 0;
    SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [&](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        if (supplierCallCount > 1) {
            progressObs->reset();
            crc->reset();
        }
        auto output = std::make_shared<std::stringstream>();
        auto writer = std::make_shared<OStreamWriter>(output);
        return std::make_shared<ObservableWriter>(writer, progressObs, crc);
    };

    auto outcome = client->getObject(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(1, supplierCallCount);
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(content.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST_F(ObservableWriterTest, GetObject_ErrorResponse_SinkNotInvoked) {
    auto client = ClientHelper::GetDefaultClient();

    int supplierCallCount = 0;
    auto crc = std::make_shared<CRC64WriteObserver>();

    SinkFactory factory;
    factory.supplier = [&](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        auto output = std::make_shared<std::stringstream>();
        auto writer = std::make_shared<OStreamWriter>(output);
        return std::make_shared<ObservableWriter>(writer, crc);
    };

    auto outcome = client->getObject(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("non-existent-key-observable-writer")
                    .setSinkFactory(factory));

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    EXPECT_EQ(0, supplierCallCount);
    EXPECT_EQ(0ULL, crc->crc());
}

TEST_F(ObservableWriterTest, GetObject_CRCOnly) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "observable-writer-crc-only";
    std::string content = "verify crc64 computation through real transport";

    auto putOutcome = client->putObject(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto output = std::make_shared<std::stringstream>();
    auto writer = std::make_shared<OStreamWriter>(output);
    auto crc = std::make_shared<CRC64WriteObserver>();

    auto sink = std::make_shared<ObservableWriter>(writer, crc);

    SinkFactory factory;
    factory.supplier = [sink](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> { return sink; };

    auto outcome = client->getObject(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content, output->str());

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
    EXPECT_NE(0ULL, crc->crc());
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
