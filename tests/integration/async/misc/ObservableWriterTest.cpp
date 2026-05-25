#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

class AsyncObservableWriterTest : public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        auto client = ClientHelper::GetDefaultClient();
        bucketName_ = Config::GenBucketName();
        auto future = client->asyncCall(models::PutBucketRequest().setBucket(bucketName_));
        EXPECT_TRUE(future.get().has_value());
    }

    static void TearDownTestCase() {
        ClientHelper::CleanBucketsByPrefix(bucketName_);
    }

  public:
    static std::string bucketName_;
};

std::string AsyncObservableWriterTest::bucketName_ = "";

TEST_F(AsyncObservableWriterTest, GetObject_WithProgressAndCRC) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-observable-writer-test";
    std::string content(128 * 1024, 'B');

    auto putFuture = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putFuture.get().has_value());

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
    factory.supplier = [sink](std::int64_t) -> std::shared_ptr<ByteWriter> { return sink; };
    factory.isOneShot = false;

    auto future = client->asyncCall(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));
    auto outcome = future.get();

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), output->str().size());
    EXPECT_EQ(content, output->str());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(content.size(), lastTransferred);
    EXPECT_EQ(content.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST_F(AsyncObservableWriterTest, GetObject_RetryWithObserverReset) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-observable-writer-retry-test";
    std::string content = "async retry content for observable writer";

    auto putFuture = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putFuture.get().has_value());

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
    factory.supplier = [&](std::int64_t) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        if (supplierCallCount > 1) {
            progressObs->reset();
            crc->reset();
        }
        auto output = std::make_shared<std::stringstream>();
        auto writer = std::make_shared<OStreamWriter>(output);
        return std::make_shared<ObservableWriter>(writer, progressObs, crc);
    };

    auto future = client->asyncCall(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));
    auto outcome = future.get();

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(1, supplierCallCount);
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(content.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST_F(AsyncObservableWriterTest, GetObject_ErrorResponse_SinkNotInvoked) {
    auto client = ClientHelper::GetDefaultClient();

    int supplierCallCount = 0;
    auto crc = std::make_shared<CRC64WriteObserver>();

    SinkFactory factory;
    factory.supplier = [&](std::int64_t) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        auto output = std::make_shared<std::stringstream>();
        auto writer = std::make_shared<OStreamWriter>(output);
        return std::make_shared<ObservableWriter>(writer, crc);
    };

    auto future = client->asyncCall(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey("non-existent-key-async-observable-writer")
                    .setSinkFactory(factory));
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    EXPECT_EQ(0, supplierCallCount);
    EXPECT_EQ(0ULL, crc->crc());
}

TEST_F(AsyncObservableWriterTest, GetObject_CRCOnly) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-observable-writer-crc-only";
    std::string content = "async verify crc64 computation through real transport";

    auto putFuture = client->asyncCall(
            models::PutObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setBody(RequestBody::FromString(content)));
    ASSERT_TRUE(putFuture.get().has_value());

    auto output = std::make_shared<std::stringstream>();
    auto writer = std::make_shared<OStreamWriter>(output);
    auto crc = std::make_shared<CRC64WriteObserver>();

    auto sink = std::make_shared<ObservableWriter>(writer, crc);

    SinkFactory factory;
    factory.supplier = [sink](std::int64_t) -> std::shared_ptr<ByteWriter> { return sink; };

    auto future = client->asyncCall(
            models::GetObjectRequest()
                    .setBucket(bucketName_)
                    .setKey(key)
                    .setSinkFactory(factory));
    auto outcome = future.get();

    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content, output->str());

    uint64_t expectedCrc = utils::CalcCRC64(0, content.data(), content.size());
    EXPECT_EQ(expectedCrc, crc->crc());
    EXPECT_NE(0ULL, crc->crc());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
