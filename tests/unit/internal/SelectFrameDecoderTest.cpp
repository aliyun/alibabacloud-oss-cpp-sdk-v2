#include <gtest/gtest.h>

#include "src/internal/SelectFrameDecoder.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <cstring>
#include <sstream>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace internal {

namespace {

void writeBE32(std::vector<uint8_t>& buf, uint32_t value) {
    buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>(value & 0xFF));
}

void writeBE64(std::vector<uint8_t>& buf, uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        buf.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

std::vector<uint8_t> buildFrame(int frameType, const std::vector<uint8_t>& payload, uint32_t checksumOverride = 0, bool useZeroChecksum = false) {
    std::vector<uint8_t> frame;

    // Version(1B) + FrameType(3B)
    frame.push_back(1); // version
    frame.push_back(static_cast<uint8_t>((frameType >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((frameType >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(frameType & 0xFF));

    // PayloadLength(4B) = payload.size() + 8 (offset)
    uint32_t payloadLength = static_cast<uint32_t>(payload.size()) + 8;
    writeBE32(frame, payloadLength);

    // HeaderChecksum(4B) - we use 0 for simplicity
    writeBE32(frame, 0);

    // Offset(8B)
    writeBE64(frame, 0);

    // PayloadData
    frame.insert(frame.end(), payload.begin(), payload.end());

    // Compute CRC32 over offset + payload
    uint32_t crc = 0;
    if (!useZeroChecksum) {
        crc = calcCRC32(0, frame.data() + 12, 8 + payload.size());
        if (checksumOverride != 0) {
            crc = checksumOverride;
        }
    }
    writeBE32(frame, crc);

    return frame;
}

} // namespace


TEST(CRC32Test, BasicComputation) {
    const char* data = "hello";
    auto crc = calcCRC32(0, data, 5);
    EXPECT_EQ(crc, 0x3610A686u);
}

TEST(CRC32Test, EmptyData) {
    auto crc = calcCRC32(0, "", 0);
    EXPECT_EQ(crc, 0u);
}

TEST(CRC32Test, Incremental) {
    const char* full = "hello world";
    auto expected = calcCRC32(0, full, 11);

    auto crc = calcCRC32(0, "hello ", 6);
    crc = calcCRC32(crc, "world", 5);
    EXPECT_EQ(crc, expected);
}


TEST(SelectFrameDecodingWriterTest, DataFrameForwardsPayload) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> payload = {'h', 'e', 'l', 'l', 'o'};
    auto frame = buildFrame(FRAME_TYPE_DATA, payload);

    decoder.write(frame.data(), frame.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("hello", ss->str());
}

TEST(SelectFrameDecodingWriterTest, ContinuousFrameIgnored) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    auto frame = buildFrame(FRAME_TYPE_CONTINUOUS, {});

    decoder.write(frame.data(), frame.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("", ss->str());
}

TEST(SelectFrameDecodingWriterTest, MultipleDataFrames) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> p1 = {'a', 'b', 'c'};
    std::vector<uint8_t> p2 = {'d', 'e', 'f'};
    auto f1 = buildFrame(FRAME_TYPE_DATA, p1);
    auto f2 = buildFrame(FRAME_TYPE_DATA, p2);

    decoder.write(f1.data(), f1.size());
    decoder.write(f2.data(), f2.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("abcdef", ss->str());
}

TEST(SelectFrameDecodingWriterTest, DataAndContinuousInterleaved) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> p1 = {'X'};
    auto f1 = buildFrame(FRAME_TYPE_DATA, p1);
    auto fc = buildFrame(FRAME_TYPE_CONTINUOUS, {});
    std::vector<uint8_t> p2 = {'Y'};
    auto f2 = buildFrame(FRAME_TYPE_DATA, p2);

    std::vector<uint8_t> all;
    all.insert(all.end(), f1.begin(), f1.end());
    all.insert(all.end(), fc.begin(), fc.end());
    all.insert(all.end(), f2.begin(), f2.end());

    decoder.write(all.data(), all.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("XY", ss->str());
}

TEST(SelectFrameDecodingWriterTest, IncrementalWrite) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> payload = {'t', 'e', 's', 't'};
    auto frame = buildFrame(FRAME_TYPE_DATA, payload);

    // Feed one byte at a time
    for (auto b : frame) {
        decoder.write(&b, 1);
    }
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("test", ss->str());
}

TEST(SelectFrameDecodingWriterTest, CrcMismatchSetsError) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> payload = {'h', 'i'};
    auto frame = buildFrame(FRAME_TYPE_DATA, payload, 0xDEADBEEF);

    decoder.write(frame.data(), frame.size());
    EXPECT_TRUE(decoder.fail());
    EXPECT_EQ("CRC32 checksum mismatch", decoder.errorMessage());
}

TEST(SelectFrameDecodingWriterTest, ZeroChecksumSkipsValidation) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> payload = {'o', 'k'};
    auto frame = buildFrame(FRAME_TYPE_DATA, payload, 0, true);

    decoder.write(frame.data(), frame.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("ok", ss->str());
}

TEST(SelectFrameDecodingWriterTest, EndFrameRecordsStatus) {
    auto ss = std::make_shared<std::stringstream>();
    auto inner = std::make_shared<OStreamWriter>(ss);
    SelectFrameDecodingWriter decoder(inner);

    std::vector<uint8_t> payload;
    writeBE32(payload, 0); // status in payload (but we read from header offset field)
    writeBE64(payload, 100); // totalScanned
    // End frame payload content doesn't matter much for the decoder,
    // status is read from header offset field

    // Build end frame with offset=0 (status comes from first 4 bytes of offset)
    auto frame = buildFrame(FRAME_TYPE_END, payload);

    decoder.write(frame.data(), frame.size());
    EXPECT_TRUE(decoder.good());
    EXPECT_EQ("", ss->str());
}


TEST(SelectMetaFrameParserTest, CsvMetaEndFrame) {
    SelectMetaFrameParser parser;

    // Build a CSV meta end frame (type 0x800006)
    // Payload: totalScanned(8B) + status(4B) + splitsCount(4B) + rowsCount(8B) + columnsCount(4B)
    std::vector<uint8_t> payload;
    writeBE64(payload, 1024);   // totalScanned
    writeBE32(payload, 200);    // status
    writeBE32(payload, 5);      // splitsCount
    writeBE64(payload, 100);    // rowsCount
    writeBE32(payload, 4);      // columnsCount

    auto frame = buildFrame(FRAME_TYPE_CSV_META_END, payload);

    parser.write(frame.data(), frame.size());
    EXPECT_TRUE(parser.good());
    EXPECT_EQ(1024, parser.totalScanned());
    EXPECT_EQ(200, parser.status());
    EXPECT_EQ(5, parser.splitsCount());
    EXPECT_EQ(100, parser.rowsCount());
    EXPECT_EQ(4, parser.columnsCount());
}

TEST(SelectMetaFrameParserTest, JsonMetaEndFrame) {
    SelectMetaFrameParser parser;

    // Build a JSON meta end frame (type 0x800007)
    // Payload: totalScanned(8B) + status(4B) + splitsCount(4B) + rowsCount(8B)
    std::vector<uint8_t> payload;
    writeBE64(payload, 2048);   // totalScanned
    writeBE32(payload, 200);    // status
    writeBE32(payload, 1);      // splitsCount
    writeBE64(payload, 50);     // rowsCount

    auto frame = buildFrame(FRAME_TYPE_JSON_META_END, payload);

    parser.write(frame.data(), frame.size());
    EXPECT_TRUE(parser.good());
    EXPECT_EQ(2048, parser.totalScanned());
    EXPECT_EQ(200, parser.status());
    EXPECT_EQ(1, parser.splitsCount());
    EXPECT_EQ(50, parser.rowsCount());
    EXPECT_EQ(0, parser.columnsCount());
}

TEST(SelectMetaFrameParserTest, CrcMismatch) {
    SelectMetaFrameParser parser;

    std::vector<uint8_t> payload;
    writeBE64(payload, 100);
    writeBE32(payload, 200);
    writeBE32(payload, 1);
    writeBE64(payload, 10);
    writeBE32(payload, 3);

    auto frame = buildFrame(FRAME_TYPE_CSV_META_END, payload, 0xBADBAD);

    parser.write(frame.data(), frame.size());
    EXPECT_TRUE(parser.fail());
}

TEST(SelectMetaFrameParserTest, ContinuousFrameIgnored) {
    SelectMetaFrameParser parser;

    auto frame = buildFrame(FRAME_TYPE_CONTINUOUS, {});
    parser.write(frame.data(), frame.size());
    EXPECT_TRUE(parser.good());

    // values should all be zero/default
    EXPECT_EQ(0, parser.totalScanned());
    EXPECT_EQ(0, parser.rowsCount());
}

TEST(SelectMetaFrameParserTest, IncrementalParsing) {
    SelectMetaFrameParser parser;

    std::vector<uint8_t> payload;
    writeBE64(payload, 512);
    writeBE32(payload, 200);
    writeBE32(payload, 2);
    writeBE64(payload, 25);
    writeBE32(payload, 3);

    auto frame = buildFrame(FRAME_TYPE_CSV_META_END, payload);

    // Feed byte by byte
    for (auto b : frame) {
        parser.write(&b, 1);
    }
    EXPECT_TRUE(parser.good());
    EXPECT_EQ(512, parser.totalScanned());
    EXPECT_EQ(200, parser.status());
    EXPECT_EQ(2, parser.splitsCount());
    EXPECT_EQ(25, parser.rowsCount());
    EXPECT_EQ(3, parser.columnsCount());
}

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
