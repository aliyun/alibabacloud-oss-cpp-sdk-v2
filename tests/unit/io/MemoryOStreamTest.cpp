#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/MemoryOStream.h"
#include "alibabacloud/oss2/Types.h"

#include <cstring>

namespace alibabacloud {
namespace oss2 {

TEST(MemoryOStreamTest, WriteBasic) {
    char buf[64] = {};
    MemoryOStream stream(buf, sizeof(buf));
    stream.write("hello", 5);
    EXPECT_TRUE(stream.good());
    EXPECT_EQ(5u, stream.written());
    EXPECT_EQ(0, std::memcmp(buf, "hello", 5));
}

TEST(MemoryOStreamTest, WriteMultiple) {
    char buf[64] = {};
    MemoryOStream stream(buf, sizeof(buf));
    stream.write("hello", 5);
    stream.write(" world", 6);
    EXPECT_TRUE(stream.good());
    EXPECT_EQ(11u, stream.written());
    EXPECT_EQ(0, std::memcmp(buf, "hello world", 11));
}

TEST(MemoryOStreamTest, WriteFull) {
    char buf[5] = {};
    MemoryOStream stream(buf, sizeof(buf));
    stream.write("12345", 5);
    EXPECT_TRUE(stream.good());
    EXPECT_EQ(5u, stream.written());
    EXPECT_EQ(0, std::memcmp(buf, "12345", 5));
}

TEST(MemoryOStreamTest, WriteOverflow) {
    char buf[4] = {};
    MemoryOStream stream(buf, sizeof(buf));
    stream.write("12345", 5);
    EXPECT_TRUE(stream.fail());
}

TEST(MemoryOStreamTest, WrittenInitiallyZero) {
    char buf[16] = {};
    MemoryOStream stream(buf, sizeof(buf));
    EXPECT_EQ(0u, stream.written());
}

TEST(MemoryOStreamTest, RetryWithNewInstance) {
    char buf[32] = {};

    // First write
    {
        MemoryOStream stream(buf, sizeof(buf));
        stream.write("first", 5);
        EXPECT_EQ(5u, stream.written());
    }

    // Simulate retry: create new instance, writes from beginning
    {
        MemoryOStream stream(buf, sizeof(buf));
        stream.write("retry", 5);
        EXPECT_EQ(5u, stream.written());
        EXPECT_EQ(0, std::memcmp(buf, "retry", 5));
    }
}

TEST(MemoryOStreamTest, WithOStreamFactory) {
    char buf[64] = {};
    std::size_t bufSize = sizeof(buf);

    OStreamFactory factory;
    factory.supplier = [ptr = buf, bufSize](std::int64_t) -> std::shared_ptr<std::ostream> {
        return std::make_shared<MemoryOStream>(ptr, bufSize);
    };
    factory.isOneShot = false;

    // First call
    auto stream1 = factory(0);
    ASSERT_NE(nullptr, stream1);
    stream1->write("hello", 5);
    EXPECT_EQ(0, std::memcmp(buf, "hello", 5));

    // Second call (simulates retry) -- starts from beginning
    auto stream2 = factory(0);
    ASSERT_NE(nullptr, stream2);
    stream2->write("world", 5);
    EXPECT_EQ(0, std::memcmp(buf, "world", 5));
}

} // namespace oss2
} // namespace alibabacloud
