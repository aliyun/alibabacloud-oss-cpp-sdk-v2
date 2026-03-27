#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteStream.h"

#include "../TestUtils.h"

#include <fstream>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

TEST(EmptyContentTest, Constructor) {
    auto content = std::make_shared<EmptyContent>();
    EXPECT_EQ(0, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());
}

TEST(EmptyContentTest, SpanSource) {
    auto content = std::make_shared<EmptyContent>();
    EXPECT_EQ(0, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    for (auto i = 0; i < 3; i++) {
        auto source = content->spanSource();
        EXPECT_TRUE(source != nullptr);

        std::string got;
        got.resize(32);
        auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), 1);
        EXPECT_EQ(0, n);

        n = source->read(reinterpret_cast<uint8_t*>(got.data()), 4);
        EXPECT_EQ(0, n);
    }
}


TEST(EmptyContentTest, SpanSourceState) {
    auto content = std::make_shared<EmptyContent>();
    EXPECT_EQ(0, content->length().value());
    EXPECT_FALSE(content->isOneShot());
    EXPECT_EQ(std::nullopt, content->path());

    auto source = content->spanSource();
    EXPECT_TRUE(source != nullptr);

    EXPECT_EQ(0, source->state());

    std::string got;
    got.resize(32);

    // read 0
    auto n = source->read(reinterpret_cast<uint8_t*>(got.data()), 0);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::goodbit, source->state());

    // no data, returns 0
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 11);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::failbit + std::ios::eofbit, source->state());

    // read 0
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 0);
    EXPECT_EQ(0, n);
    EXPECT_EQ(std::ios::failbit + std::ios::eofbit, source->state());
}

} // namespace oss2
} // namespace alibabacloud