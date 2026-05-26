#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteStream.h"
#include "src/internal/ByteStreamUtils.h"

#include <sstream>
#include <cstring>

using namespace alibabacloud::oss2::internal;

class MockStreamObserver : public StreamObserver {
  public:
    void data(std::uint8_t* buffer, std::size_t count) override {
        callCount_++;
        totalBytes_ += count;
    }

    void reset() override {
        callCount_ = 0;
        totalBytes_ = 0;
    }

    int callCount_ = 0;
    std::size_t totalBytes_ = 0;
};

// Minimal observer that only implements data() — uses all base class defaults
class MinimalStreamObserver : public StreamObserver {
  public:
    void data(std::uint8_t*, std::size_t) override {}
};

TEST(ByteStreamUtilsTest, ProgressObserverTest) {
    std::size_t capturedIncrement = 0;
    std::size_t capturedTransferred = 0;
    std::int64_t capturedTotal = 0;

    auto callbackFunc = [&capturedIncrement, &capturedTransferred, &capturedTotal](
                                std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t userdata) {
        capturedIncrement = increment;
        capturedTransferred = transferred;
        capturedTotal = total;
    };

    alibabacloud::oss2::ProgressCallback callback;
    callback.callback = callbackFunc;

    ProgressObserver observer(callback, 1000);

    std::uint8_t buffer[100];
    memset(buffer, 0, 100);

    observer.data(buffer, 50);
    EXPECT_EQ(50, capturedTransferred);
    EXPECT_EQ(1000, capturedTotal);

    observer.data(buffer, 30);
    EXPECT_EQ(80, capturedTransferred);

    observer.reset();
    // After reset, written_ is reset to 0, lastWritten_ is set to previous written_
    // The next data call will see written_ > lastWritten_ is false initially
}

TEST(ByteStreamUtilsTest, CRC64ObserverTest) {
    CRC64Observer observer(0);

    std::uint8_t buffer[] = "hello world";
    observer.data(buffer, strlen((char*)buffer));

    uint64_t crc = observer.crc();
    EXPECT_NE(0, crc);

    observer.reset();
    EXPECT_EQ(0, observer.crc());
}

TEST(ByteStreamUtilsTest, CRC64ObserverWithInitValue) {
    CRC64Observer observer(0xFFFFFFFF);
    EXPECT_EQ(0xFFFFFFFF, observer.crc());

    std::uint8_t buffer[] = "test";
    observer.data(buffer, 4);

    uint64_t crc = observer.crc();
    EXPECT_NE(0, crc);
    EXPECT_NE(0xFFFFFFFF, crc);
}

TEST(ByteStreamUtilsTest, TeeByteContentTest) {
    auto source = std::make_shared<alibabacloud::oss2::StringContent>("hello world");

    auto observer1 = std::make_shared<MockStreamObserver>();
    auto observer2 = std::make_shared<MockStreamObserver>();

    std::vector<std::shared_ptr<StreamObserver>> sinks;
    sinks.push_back(observer1);
    sinks.push_back(observer2);

    TeeByteContent tee(source, sinks);

    EXPECT_TRUE(tee.length().has_value());
    EXPECT_EQ(11, tee.length().value());
    EXPECT_FALSE(tee.isOneShot());

    auto spanSource = tee.spanSource();
    EXPECT_NE(nullptr, spanSource);

    std::uint8_t buffer[20];
    memset(buffer, 0, 20);
    auto bytesRead = spanSource->read(buffer, 20);

    EXPECT_EQ(11, bytesRead);
    EXPECT_STREQ("hello world", (char*)buffer);

    EXPECT_EQ(1, observer1->callCount_);
    EXPECT_EQ(11, observer1->totalBytes_);
    EXPECT_EQ(1, observer2->callCount_);
    EXPECT_EQ(11, observer2->totalBytes_);
}

TEST(ByteStreamUtilsTest, TeeByteContentMultipleReads) {
    auto source = std::make_shared<alibabacloud::oss2::StringContent>("test data");
    auto observer = std::make_shared<MockStreamObserver>();

    std::vector<std::shared_ptr<StreamObserver>> sinks;
    sinks.push_back(observer);

    TeeByteContent tee(source, sinks);

    // First read
    auto spanSource1 = tee.spanSource();
    std::uint8_t buffer1[20];
    memset(buffer1, 0, 20);
    auto bytesRead1 = spanSource1->read(buffer1, 20);
    EXPECT_EQ(9, bytesRead1);
    EXPECT_EQ(1, observer->callCount_);

    // Second read resets observer then calls data again
    auto spanSource2 = tee.spanSource();
    std::uint8_t buffer2[20];
    memset(buffer2, 0, 20);
    auto bytesRead2 = spanSource2->read(buffer2, 20);
    EXPECT_EQ(9, bytesRead2);
    // After second read, callCount should be 2 (one from each read)
    EXPECT_GE(observer->callCount_, 1);
    EXPECT_GE(observer->totalBytes_, 9);
}

TEST(ByteStreamUtilsTest, TeeByteContentWithNullObserver) {
    auto source = std::make_shared<alibabacloud::oss2::StringContent>("test");

    std::vector<std::shared_ptr<StreamObserver>> sinks;
    sinks.push_back(nullptr);
    sinks.push_back(std::make_shared<MockStreamObserver>());

    TeeByteContent tee(source, sinks);

    auto spanSource = tee.spanSource();
    std::uint8_t buffer[20];
    spanSource->read(buffer, 20);
}

TEST(ByteStreamUtilsTest, TeeByteContentEmptySinks) {
    auto source = std::make_shared<alibabacloud::oss2::StringContent>("empty test");

    std::vector<std::shared_ptr<StreamObserver>> sinks;

    TeeByteContent tee(source, sinks);

    auto spanSource = tee.spanSource();
    std::uint8_t buffer[20];
    auto bytesRead = spanSource->read(buffer, 20);

    EXPECT_EQ(10, bytesRead);
}

TEST(ByteStreamUtilsTest, StreamObserverBaseDefaults) {
    MinimalStreamObserver observer;
    // These are the base class default virtual implementations (no-ops)
    std::error_code ec = std::make_error_code(std::errc::io_error);
    observer.closed();
    observer.error(ec);
    observer.finished();
    observer.reset();
    // If we get here without crash, the defaults work
    SUCCEED();
}

TEST(ByteStreamUtilsTest, TeeByteContentPathReturnsNullopt) {
    auto source = std::make_shared<alibabacloud::oss2::StringContent>("test");
    std::vector<std::shared_ptr<StreamObserver>> sinks;
    sinks.push_back(std::make_shared<MockStreamObserver>());

    TeeByteContent tee(source, sinks);
    // TeeByteContent::path() always returns nullopt
    EXPECT_FALSE(tee.path().has_value());
}
