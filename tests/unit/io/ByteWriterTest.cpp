#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

#include <cstring>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

namespace {
const std::uint8_t* u8(const char* s) { return reinterpret_cast<const std::uint8_t*>(s); }

class RecordingObserver : public ByteWriterObserver {
  public:
    std::string data;
    int resetCount{0};

  private:
    std::size_t onWrite(const std::uint8_t* d, std::size_t n) override {
        data.append(reinterpret_cast<const char*>(d), n);
        return n;
    }
    void onReset() override { ++resetCount; data.clear(); }
};
}

TEST(ObservableWriterTest, WriteForwardsToWriterAndObservers) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);
    auto obs1 = std::make_shared<RecordingObserver>();
    auto obs2 = std::make_shared<RecordingObserver>();

    ObservableWriter ow(writer, obs1, obs2);
    ow.write(u8("hello"), 5);

    EXPECT_EQ("hello", mainStream->str());
    EXPECT_EQ("hello", obs1->data);
    EXPECT_EQ("hello", obs2->data);
}

TEST(ObservableWriterTest, MultipleWrites) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);
    auto obs = std::make_shared<RecordingObserver>();

    ObservableWriter ow(writer, obs);
    ow.write(u8("abc"), 3);
    ow.write(u8("def"), 3);

    EXPECT_EQ("abcdef", mainStream->str());
    EXPECT_EQ("abcdef", obs->data);
}

TEST(ObservableWriterTest, NoObservers) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);

    ObservableWriter ow(writer);
    ow.write(u8("data"), 4);

    EXPECT_EQ("data", mainStream->str());
    EXPECT_EQ(0, ow.state());
}

TEST(ObservableWriterTest, StateFromWriter) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);

    ObservableWriter ow(writer);
    EXPECT_EQ(0, ow.state());
}

TEST(ObservableWriterTest, ResetObserversDirectly) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);
    auto obs1 = std::make_shared<RecordingObserver>();
    auto obs2 = std::make_shared<RecordingObserver>();

    ObservableWriter ow(writer, obs1, obs2);
    ow.write(u8("hello"), 5);
    obs1->reset();
    obs2->reset();

    EXPECT_EQ(1, obs1->resetCount);
    EXPECT_EQ(1, obs2->resetCount);
    EXPECT_EQ("", obs1->data);
    EXPECT_EQ("", obs2->data);
}

TEST(ObservableWriterTest, WithProgressAndCRC) {
    auto mainStream = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(mainStream);

    std::size_t totalTransferred = 0;
    ProgressCallback cb;
    cb.callback = [&](std::size_t, std::size_t transferred,
                      std::int64_t, std::uintptr_t) {
        totalTransferred = transferred;
    };
    auto progress = std::make_shared<ProgressWriteObserver>(cb, 11);
    auto crc = std::make_shared<CRC64WriteObserver>();

    ObservableWriter ow(writer, progress, crc);
    ow.write(u8("hello world"), 11);

    EXPECT_EQ("hello world", mainStream->str());
    EXPECT_EQ(11u, totalTransferred);

    std::string data = "hello world";
    uint64_t expected = utils::calcCRC64(0, data.data(), data.size());
    EXPECT_EQ(expected, crc->crc());
}

TEST(ProgressWriteObserverTest, TracksProgress) {
    std::size_t lastIncrement = 0;
    std::size_t lastTransferred = 0;
    std::int64_t lastTotal = 0;

    ProgressCallback cb;
    cb.callback = [&](std::size_t increment, std::size_t transferred,
                      std::int64_t total, std::uintptr_t) {
        lastIncrement = increment;
        lastTransferred = transferred;
        lastTotal = total;
    };

    ProgressWriteObserver progress(cb, 100);
    progress.write(u8("12345"), 5);

    EXPECT_EQ(5u, lastIncrement);
    EXPECT_EQ(5u, lastTransferred);
    EXPECT_EQ(100, lastTotal);

    progress.write(u8("67890"), 5);
    EXPECT_EQ(5u, lastIncrement);
    EXPECT_EQ(10u, lastTransferred);
    EXPECT_EQ(100, lastTotal);
}

TEST(ProgressWriteObserverTest, Reset) {
    std::size_t lastTransferred = 0;
    int callCount = 0;
    ProgressCallback cb;
    cb.callback = [&](std::size_t, std::size_t transferred,
                      std::int64_t, std::uintptr_t) {
        lastTransferred = transferred;
        ++callCount;
    };

    ProgressWriteObserver progress(cb, 100);
    progress.write(u8("12345"), 5);
    EXPECT_EQ(5u, lastTransferred);
    EXPECT_EQ(1, callCount);

    progress.reset();

    // after reset, callback suppressed until past previous high-water mark
    progress.write(u8("abc"), 3);
    EXPECT_EQ(1, callCount);

    // once past previous transferred (5), callback fires again
    progress.write(u8("defgh"), 5);
    EXPECT_EQ(2, callCount);
    EXPECT_EQ(8u, lastTransferred);
}

TEST(CRC64WriteObserverTest, EmptyWriter) {
    CRC64WriteObserver crc;
    EXPECT_EQ(0u, crc.crc());
}

TEST(CRC64WriteObserverTest, ComputesCRC) {
    std::string data = "hello world";

    CRC64WriteObserver crc;
    crc.write(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());

    uint64_t expected = utils::calcCRC64(0, data.data(), data.size());
    EXPECT_EQ(expected, crc.crc());
}

TEST(CRC64WriteObserverTest, IncrementalMatchesFull) {
    std::string part1 = "hello ";
    std::string part2 = "world";
    std::string full = part1 + part2;

    CRC64WriteObserver crc;
    crc.write(reinterpret_cast<const std::uint8_t*>(part1.data()), part1.size());
    crc.write(reinterpret_cast<const std::uint8_t*>(part2.data()), part2.size());

    uint64_t expected = utils::calcCRC64(0, full.data(), full.size());
    EXPECT_EQ(expected, crc.crc());
}

TEST(CRC64WriteObserverTest, Reset) {
    std::string data = "hello world";

    CRC64WriteObserver crc;
    crc.write(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
    EXPECT_NE(0u, crc.crc());

    crc.reset();
    EXPECT_EQ(0u, crc.crc());
}

TEST(OStreamWriterTest, WritesAndReportsState) {
    auto ss = std::make_shared<std::ostringstream>();
    OStreamWriter writer(ss);

    EXPECT_EQ(0, writer.state());
    writer.write(u8("test"), 4);
    EXPECT_EQ("test", ss->str());
    EXPECT_EQ(0, writer.state());
}

TEST(ByteWriterTest, GoodBadFailMatchIostream) {
    auto ss = std::make_shared<std::stringstream>();
    OStreamWriter writer(ss);

    // initial state: good
    EXPECT_TRUE(writer.good());
    EXPECT_FALSE(writer.fail());
    EXPECT_FALSE(writer.bad());
    EXPECT_TRUE(ss->good());
    EXPECT_EQ(writer.good(), ss->good());
    EXPECT_EQ(writer.fail(), ss->fail());
    EXPECT_EQ(writer.bad(), ss->bad());

    // set failbit
    ss->setstate(std::ios_base::failbit);
    EXPECT_FALSE(writer.good());
    EXPECT_TRUE(writer.fail());
    EXPECT_FALSE(writer.bad());
    EXPECT_EQ(writer.good(), ss->good());
    EXPECT_EQ(writer.fail(), ss->fail());
    EXPECT_EQ(writer.bad(), ss->bad());

    // clear and set badbit
    ss->clear();
    ss->setstate(std::ios_base::badbit);
    EXPECT_FALSE(writer.good());
    EXPECT_TRUE(writer.fail());
    EXPECT_TRUE(writer.bad());
    EXPECT_EQ(writer.good(), ss->good());
    EXPECT_EQ(writer.fail(), ss->fail());
    EXPECT_EQ(writer.bad(), ss->bad());

    // clear and set both
    ss->clear();
    ss->setstate(std::ios_base::failbit | std::ios_base::badbit);
    EXPECT_FALSE(writer.good());
    EXPECT_TRUE(writer.fail());
    EXPECT_TRUE(writer.bad());
    EXPECT_EQ(writer.good(), ss->good());
    EXPECT_EQ(writer.fail(), ss->fail());
    EXPECT_EQ(writer.bad(), ss->bad());

    // clear back to good
    ss->clear();
    EXPECT_TRUE(writer.good());
    EXPECT_FALSE(writer.fail());
    EXPECT_FALSE(writer.bad());
    EXPECT_EQ(writer.good(), ss->good());
    EXPECT_EQ(writer.fail(), ss->fail());
    EXPECT_EQ(writer.bad(), ss->bad());
}

TEST(ByteWriterTest, ObserverAlwaysGood) {
    CRC64WriteObserver crc;
    EXPECT_TRUE(crc.good());
    EXPECT_FALSE(crc.fail());
    EXPECT_FALSE(crc.bad());

    ProgressCallback cb;
    cb.callback = [](std::size_t, std::size_t, std::int64_t, std::uintptr_t) {};
    ProgressWriteObserver progress(cb, 100);
    EXPECT_TRUE(progress.good());
    EXPECT_FALSE(progress.fail());
    EXPECT_FALSE(progress.bad());
}

TEST(MemoryWriterTest, WriteBasic) {
    std::uint8_t buf[64] = {};
    MemoryWriter writer(buf, sizeof(buf));
    auto n = writer.write(u8("hello"), 5);
    EXPECT_EQ(5u, n);
    EXPECT_TRUE(writer.good());
    EXPECT_EQ(5u, writer.written());
    EXPECT_EQ(0, std::memcmp(buf, "hello", 5));
}

TEST(MemoryWriterTest, WriteMultiple) {
    std::uint8_t buf[64] = {};
    MemoryWriter writer(buf, sizeof(buf));
    writer.write(u8("hello"), 5);
    writer.write(u8(" world"), 6);
    EXPECT_TRUE(writer.good());
    EXPECT_EQ(11u, writer.written());
    EXPECT_EQ(0, std::memcmp(buf, "hello world", 11));
}

TEST(MemoryWriterTest, WriteFull) {
    std::uint8_t buf[5] = {};
    MemoryWriter writer(buf, sizeof(buf));
    auto n = writer.write(u8("12345"), 5);
    EXPECT_EQ(5u, n);
    EXPECT_TRUE(writer.good());
    EXPECT_EQ(5u, writer.written());
    EXPECT_EQ(0, std::memcmp(buf, "12345", 5));
}

TEST(MemoryWriterTest, WriteOverflow) {
    std::uint8_t buf[4] = {};
    MemoryWriter writer(buf, sizeof(buf));
    auto n = writer.write(u8("12345"), 5);
    EXPECT_EQ(0u, n);
    EXPECT_TRUE(writer.fail());
    EXPECT_FALSE(writer.good());
}

TEST(MemoryWriterTest, WrittenInitiallyZero) {
    std::uint8_t buf[16] = {};
    MemoryWriter writer(buf, sizeof(buf));
    EXPECT_EQ(0u, writer.written());
    EXPECT_TRUE(writer.good());
}

TEST(MemoryWriterTest, RetryWithNewInstance) {
    std::uint8_t buf[32] = {};

    {
        MemoryWriter writer(buf, sizeof(buf));
        writer.write(u8("first"), 5);
        EXPECT_EQ(5u, writer.written());
    }

    {
        MemoryWriter writer(buf, sizeof(buf));
        writer.write(u8("retry"), 5);
        EXPECT_EQ(5u, writer.written());
        EXPECT_EQ(0, std::memcmp(buf, "retry", 5));
    }
}

TEST(MemoryWriterTest, WithSinkFactory) {
    std::uint8_t buf[64] = {};
    std::size_t bufSize = sizeof(buf);

    auto factory = makeSinkFactory([ptr = buf, bufSize](std::int64_t) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<MemoryWriter>(ptr, bufSize);
    });

    auto writer1 = factory(0, HeaderCollection{});
    ASSERT_NE(nullptr, writer1);
    writer1->write(u8("hello"), 5);
    EXPECT_EQ(0, std::memcmp(buf, "hello", 5));

    auto writer2 = factory(0, HeaderCollection{});
    ASSERT_NE(nullptr, writer2);
    writer2->write(u8("world"), 5);
    EXPECT_EQ(0, std::memcmp(buf, "world", 5));
}

} // namespace oss2
} // namespace alibabacloud
