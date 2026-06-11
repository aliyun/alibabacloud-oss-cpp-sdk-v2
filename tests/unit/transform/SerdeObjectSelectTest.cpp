#include <gtest/gtest.h>

#include "src/internal/SelectFrameDecoder.h"
#include "src/transform/SerdeObjectSelect.h"

#include <sstream>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace transform {

namespace {
std::string bodyToString(const std::shared_ptr<ByteContent>& body) {
    auto src = body->spanSource();
    auto d = src->readToEnd();
    return std::string(d.begin(), d.end());
}
} // namespace

TEST(SerdeObjectSelectTest, FromSelectObjectBasicCSV) {
    models::SelectObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test.csv");
    request.setProcess("csv/select");

    models::SelectRequest selectReq;
    selectReq.setExpression("c2VsZWN0ICogZnJvbSBvc3NvYmplY3Q=");  // base64 of "select * from ossobject"

    models::InputSerialization input;
    input.setCompressionType("NONE");
    models::CSVInputFormat csvInput;
    csvInput.setFileHeaderInfo("Use");
    csvInput.setRecordDelimiter("Cg==");  // base64 of "\n"
    csvInput.setFieldDelimiter("LA==");   // base64 of ","
    input.setCsv(csvInput);
    selectReq.setInputSerialization(input);

    models::OutputSerialization output;
    models::CSVOutputFormat csvOutput;
    csvOutput.setRecordDelimiter("Cg==");
    csvOutput.setFieldDelimiter("LA==");
    output.setCsv(csvOutput);
    output.setKeepAllColumns(false);
    output.setOutputRawData(false);
    output.setEnablePayloadCrc(true);
    selectReq.setOutputSerialization(output);

    request.setSelectRequest(selectReq);

    auto opInput = fromSelectObject(request);
    EXPECT_EQ("SelectObject", opInput.opName);
    EXPECT_EQ("POST", opInput.method);
    EXPECT_EQ("test-bucket", opInput.bucket);
    EXPECT_EQ("test.csv", opInput.key);
    EXPECT_EQ("csv/select", opInput.parameters["x-oss-process"]);

    ASSERT_NE(nullptr, opInput.body);
    auto bodyStr = bodyToString(opInput.body);
    EXPECT_NE(std::string::npos, bodyStr.find("<SelectRequest>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<Expression>c2VsZWN0ICogZnJvbSBvc3NvYmplY3Q=</Expression>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<CompressionType>NONE</CompressionType>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<FileHeaderInfo>Use</FileHeaderInfo>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<RecordDelimiter>Cg==</RecordDelimiter>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<FieldDelimiter>LA==</FieldDelimiter>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<KeepAllColumns>false</KeepAllColumns>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<OutputRawData>false</OutputRawData>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<EnablePayloadCrc>true</EnablePayloadCrc>"));
    EXPECT_NE(std::string::npos, bodyStr.find("</SelectRequest>"));
}

TEST(SerdeObjectSelectTest, FromSelectObjectJSON) {
    models::SelectObjectRequest request;
    request.setBucket("test-bucket");
    request.setKey("test.json");
    request.setProcess("json/select");

    models::SelectRequest selectReq;
    selectReq.setExpression("c2VsZWN0ICogZnJvbSBvc3NvYmplY3Q=");

    models::InputSerialization input;
    models::JSONInputFormat jsonInput;
    jsonInput.setType("LINES");
    jsonInput.setParseJsonNumberAsString(true);
    input.setJson(jsonInput);
    selectReq.setInputSerialization(input);

    models::OutputSerialization output;
    models::JSONOutputFormat jsonOutput;
    jsonOutput.setRecordDelimiter("Cg==");
    output.setJson(jsonOutput);
    selectReq.setOutputSerialization(output);

    request.setSelectRequest(selectReq);

    auto opInput = fromSelectObject(request);
    EXPECT_EQ("json/select", opInput.parameters["x-oss-process"]);

    auto bodyStr = bodyToString(opInput.body);
    EXPECT_NE(std::string::npos, bodyStr.find("<Type>LINES</Type>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<ParseJsonNumberAsString>true</ParseJsonNumberAsString>"));
}

TEST(SerdeObjectSelectTest, FromSelectObjectWithOptions) {
    models::SelectObjectRequest request;
    request.setBucket("b");
    request.setKey("k");
    request.setProcess("csv/select");

    models::SelectRequest selectReq;
    selectReq.setExpression("dGVzdA==");

    models::SelectRequestOptions opts;
    opts.setSkipPartialDataRecord(true);
    opts.setMaxSkippedRecordsAllowed(100);
    selectReq.setOptions(opts);

    request.setSelectRequest(selectReq);

    auto opInput = fromSelectObject(request);
    auto bodyStr = bodyToString(opInput.body);
    EXPECT_NE(std::string::npos, bodyStr.find("<SkipPartialDataRecord>true</SkipPartialDataRecord>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<MaxSkippedRecordsAllowed>100</MaxSkippedRecordsAllowed>"));
}

TEST(SerdeObjectSelectTest, FromCreateSelectObjectMetaCSV) {
    models::CreateSelectObjectMetaRequest request;
    request.setBucket("test-bucket");
    request.setKey("test.csv");
    request.setProcess("csv/meta");

    models::CSVMetaRequest metaReq;
    models::InputSerialization input;
    input.setCompressionType("NONE");
    models::CSVInputFormat csvInput;
    csvInput.setRecordDelimiter("Cg==");
    csvInput.setFieldDelimiter("LA==");
    csvInput.setQuoteCharacter("Ig==");
    input.setCsv(csvInput);
    metaReq.setInputSerialization(input);
    metaReq.setOverwriteIfExists(true);
    request.setSelectMetaRequest(metaReq);

    auto opInput = fromCreateSelectObjectMeta(request);
    EXPECT_EQ("CreateSelectObjectMeta", opInput.opName);
    EXPECT_EQ("POST", opInput.method);
    EXPECT_EQ("test-bucket", opInput.bucket);
    EXPECT_EQ("test.csv", opInput.key);
    EXPECT_EQ("csv/meta", opInput.parameters["x-oss-process"]);

    auto bodyStr = bodyToString(opInput.body);
    EXPECT_NE(std::string::npos, bodyStr.find("<CsvMetaRequest>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<CompressionType>NONE</CompressionType>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<RecordDelimiter>Cg==</RecordDelimiter>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<OverwriteIfExists>true</OverwriteIfExists>"));
    EXPECT_NE(std::string::npos, bodyStr.find("</CsvMetaRequest>"));
}

TEST(SerdeObjectSelectTest, FromCreateSelectObjectMetaJSON) {
    models::CreateSelectObjectMetaRequest request;
    request.setBucket("test-bucket");
    request.setKey("test.json");
    request.setProcess("json/meta");

    models::JSONMetaRequest metaReq;
    models::InputSerialization input;
    models::JSONInputFormat jsonInput;
    jsonInput.setType("LINES");
    input.setJson(jsonInput);
    metaReq.setInputSerialization(input);
    metaReq.setOverwriteIfExists(false);
    request.setSelectMetaRequest(metaReq);

    auto opInput = fromCreateSelectObjectMeta(request);
    EXPECT_EQ("json/meta", opInput.parameters["x-oss-process"]);

    auto bodyStr = bodyToString(opInput.body);
    EXPECT_NE(std::string::npos, bodyStr.find("<JsonMetaRequest>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<Type>LINES</Type>"));
    EXPECT_NE(std::string::npos, bodyStr.find("<OverwriteIfExists>false</OverwriteIfExists>"));
    EXPECT_NE(std::string::npos, bodyStr.find("</JsonMetaRequest>"));
}

TEST(SerdeObjectSelectTest, ToSelectObject) {
    OperationOutput output;
    output.statusCode = 206;
    output.headers["x-oss-request-id"] = "req-123";

    auto outcome = toSelectObject(std::move(output));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());
    EXPECT_EQ("req-123", outcome.value().getRequestId());
}

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaNoBody) {
    OperationOutput output;
    output.statusCode = 200;
    output.headers["x-oss-request-id"] = "req-456";

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(0, outcome.value().getRowsCount());
}

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
std::vector<uint8_t> buildMetaFrame(int frameType, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame;
    frame.push_back(1);
    frame.push_back(static_cast<uint8_t>((frameType >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((frameType >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(frameType & 0xFF));
    uint32_t payloadLength = static_cast<uint32_t>(payload.size()) + 8;
    writeBE32(frame, payloadLength);
    writeBE32(frame, 0);
    writeBE64(frame, 0);
    frame.insert(frame.end(), payload.begin(), payload.end());
    uint32_t crc = internal::calcCRC32(0, frame.data() + 12, 8 + payload.size());
    writeBE32(frame, crc);
    return frame;
}
} // namespace

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaWithBody) {
    // Build a CSV meta end frame
    std::vector<uint8_t> payload;
    writeBE64(payload, 2048);  // totalScanned
    writeBE32(payload, 200);   // status
    writeBE32(payload, 3);     // splitsCount
    writeBE64(payload, 50);    // rowsCount
    writeBE32(payload, 5);     // columnsCount
    auto frame = buildMetaFrame(internal::FRAME_TYPE_CSV_META_END, payload);

    auto ss = std::make_shared<std::stringstream>();
    ss->write(reinterpret_cast<const char*>(frame.data()), frame.size());
    ss->seekg(0);

    OperationOutput output;
    output.statusCode = 200;
    output.headers["x-oss-request-id"] = "req-789";
    output.body = ss;

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(2048, outcome.value().getTotalScanned());
    EXPECT_EQ(200, outcome.value().getStatus());
    EXPECT_EQ(3, outcome.value().getSplitsCount());
    EXPECT_EQ(50, outcome.value().getRowsCount());
    EXPECT_EQ(5, outcome.value().getColumnsCount());
}

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaPayloadChecksumFail) {
    // Ported from v1: CreateSelectObjectMetaWithPayloadChecksumFailTest
    // CSV meta end frame (0x800006) with invalid CRC checksum
    unsigned char data[] = {
        0x01, 0x80, 0x00, 0x06, 0x00, 0x00, 0x00, 0x25,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0xc8,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04,
        0x2e, 0x78, 0x95, 0x1f, 0x00};

    auto ss = std::make_shared<std::stringstream>();
    ss->write(reinterpret_cast<const char*>(data), sizeof(data));
    ss->seekg(0);

    OperationOutput output;
    output.statusCode = 200;
    output.body = ss;

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("SelectMetaFrameError", outcome.error().getCode());
    EXPECT_EQ("CRC32 checksum mismatch", outcome.error().getMessage());
}

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaTruncatedFrame) {
    // Ported from v1: CreateSelectObjectMetaWithParseIOStreamFailTest
    // Truncated frame (only 40 bytes, missing tail and part of payload)
    unsigned char data[] = {
        0x01, 0x80, 0x00, 0x06, 0x00, 0x00, 0x00, 0x25,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0xc8,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};

    auto ss = std::make_shared<std::stringstream>();
    ss->write(reinterpret_cast<const char*>(data), sizeof(data));
    ss->seekg(0);

    OperationOutput output;
    output.statusCode = 200;
    output.body = ss;

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    // Truncated frame: parser never completes, returns default values
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(0, outcome.value().getRowsCount());
}

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaUnknownFrameType) {
    // Ported from v1: CreateSelectObjectMetaRequestValidateTest (unknown frame type 0x800008)
    unsigned char data[] = {
        0x01, 0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x25,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0xc8,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};

    auto ss = std::make_shared<std::stringstream>();
    ss->write(reinterpret_cast<const char*>(data), sizeof(data));
    ss->seekg(0);

    OperationOutput output;
    output.statusCode = 200;
    output.body = ss;

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    // Unknown frame type with truncated data: parser doesn't extract metadata
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(0, outcome.value().getRowsCount());
}

TEST(SerdeObjectSelectTest, ToCreateSelectObjectMetaInvalidResponseBody) {
    // Ported from v1: CreateSelectObjectMetaWithInvalidResponseBodyTest
    // Complete frame with non-zero but incorrect checksum (0xDEADBEEF)
    unsigned char data[] = {
        0x01, 0x80, 0x00, 0x06, 0x00, 0x00, 0x00, 0x24,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0xc8,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04,
        0xDE, 0xAD, 0xBE, 0xEF};

    auto ss = std::make_shared<std::stringstream>();
    ss->write(reinterpret_cast<const char*>(data), sizeof(data));
    ss->seekg(0);

    OperationOutput output;
    output.statusCode = 200;
    output.body = ss;

    auto outcome = toCreateSelectObjectMeta(std::move(output));
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("SelectMetaFrameError", outcome.error().getCode());
    EXPECT_EQ("CRC32 checksum mismatch", outcome.error().getMessage());
}

} // namespace transform
} // namespace oss2
} // namespace alibabacloud
