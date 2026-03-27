#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteStream.h"

#include "../TestUtils.h"

#include <fstream>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

TEST(MemoryContentTest, Constructor) {
    std::string data = "";
    auto content = std::make_shared<MemoryContent>(data);
    EXPECT_EQ(0, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    data = "hello world";
    content = std::make_shared<MemoryContent>(data);
    EXPECT_EQ(11, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    const char* data2 = "hello oss!";
    content = std::make_shared<MemoryContent>(data2);
    EXPECT_EQ(10, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    content = std::make_shared<MemoryContent>(data2, 5);
    EXPECT_EQ(5, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());
}

TEST(MemoryContentTest, SpanSource) {
    const std::string data = "hello world";
    auto content = std::make_shared<MemoryContent>(data);
    EXPECT_EQ(11, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    for (auto i = 0; i < 3; i++) {
        auto source = content->spanSource();
        EXPECT_TRUE(source != nullptr);

        std::string got;
        got.resize(32);
        auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), 1);
        EXPECT_EQ(1, n);
        EXPECT_EQ(data.substr(0, 1), got.substr(0, 1));

        n = source->read(reinterpret_cast<uint8_t*>(got.data()), 4);
        EXPECT_EQ(4, n);
        EXPECT_EQ(data.substr(1, 4), got.substr(0, 4));

        n = source->read(reinterpret_cast<uint8_t*>(got.data()), 11);
        EXPECT_EQ(6, n);
        EXPECT_EQ(data.substr(1 + 4), got.substr(0, n));

        // no data, returns 0
        n = source->read(reinterpret_cast<uint8_t*>(got.data()), 11);
        EXPECT_EQ(0, n);
    }
}

TEST(MemoryContentTest, SpanSourceState) {
    const std::string data = "hello world";
    auto content = std::make_shared<MemoryContent>(data);
    EXPECT_EQ(11, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    auto source = content->spanSource();
    EXPECT_TRUE(source != nullptr);

    EXPECT_EQ(0, source->state());

    std::string got;
    got.resize(32);

    // remains data
    auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), 1);
    EXPECT_EQ(1, n);
    EXPECT_EQ(data.substr(0, 1), got.substr(0, 1));
    EXPECT_EQ(0, source->state());

    // read to end
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 10);
    EXPECT_EQ(10, n);
    EXPECT_EQ(data.substr(1, 10), got.substr(0, 10));
    EXPECT_EQ(std::ios::goodbit, source->state());

    // read 0
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 0);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::goodbit, source->state());

    // no data, returns 0
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 11);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::failbit + std::ios::eofbit, source->state());

    // read len > data len
    source = content->spanSource();
    EXPECT_TRUE(source != nullptr);
    EXPECT_EQ(0, source->state());

    // read to end
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 12);
    EXPECT_EQ(11, n);
    EXPECT_EQ(data.substr(0, 11), got.substr(0, 11));
    EXPECT_EQ(std::ios::failbit + std::ios::eofbit, source->state());

    // read 0
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 0);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::failbit + std::ios::eofbit, source->state());
}


TEST(MemoryContentTest, SpanSourceRandom) {
    std::size_t len = 1024 + 1234;
    auto data = TestUtils::GenRandomString(len);
    EXPECT_EQ(len, data.size());

    auto content = std::make_shared<MemoryContent>(data);
    EXPECT_EQ(len, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    auto source = content->spanSource();
    std::string got;
    got.resize(32);
    size_t i;
    for (i = 0; i < len;) {
        size_t remains = std::rand() % 16;
        remains = std::min(remains, len - i);
        auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), remains);
        EXPECT_EQ(remains, n);
        EXPECT_EQ(data.substr(i, n), got.substr(0, n));
        i += remains;
    }
    EXPECT_EQ(i, len);

    // read all
    source = content->spanSource();
    got.resize(len);
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), len + 100);
    EXPECT_EQ(len, n);
    EXPECT_EQ(data, got.substr(0, n));

    // const string
    const std::string cosntData = data;
    content = std::make_shared<MemoryContent>(cosntData);
    source = content->spanSource();
    for (i = 0; i < len;) {
        size_t remains = std::rand() % 23;
        remains = std::min(remains, len - i);
        auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), remains);
        EXPECT_EQ(remains, n);
        EXPECT_EQ(data.substr(i, n), got.substr(0, n));
        i += remains;
    }
    EXPECT_EQ(i, len);

    // read part
    std::string got1;
    got1.resize(1234);
    source = content->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got1.data()), 123);
    EXPECT_EQ(123, n);
    EXPECT_EQ(data.substr(0, 123), got1.substr(0, 123));
}

} // namespace oss2
} // namespace alibabacloud