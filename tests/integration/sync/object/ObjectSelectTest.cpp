#include <gtest/gtest.h>

#include "Config.h"
#include "sync/ClientHelper.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"
#include "src/utils/Utils.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace sync {

using utils::base64Encode;

static std::string streamToString(const std::shared_ptr<std::iostream>& s) {
    if (!s) return "";
    std::ostringstream ss;
    ss << s->rdbuf();
    return ss.str();
}

class ObjectSelectTest : public ::testing::Test {
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

    static const std::string& csvData() {
        static std::string data =
            "name,school,company,age\n"
            "Lora Francis,School,Staples Inc,27\n"
            "#Lora Francis,School,Staples Inc,27\n"
            "Eleanor Little,School,\"Conectiv, Inc\",43\n"
            "Rosie Hughes,School,Western Gas Resources Inc,44\n"
            "Lawrence Ross,School,MetLife Inc.,24\n";
        return data;
    }

    static const std::string& csvDataCRLF() {
        static std::string data =
            "name,school,company,age\r\n"
            "Lora Francis,School A,Staples Inc,27\r\n"
            "Eleanor Little,School B,\"Conectiv, Inc\",43\r\n"
            "Rosie Hughes,School C,Western Gas Resources Inc,44\r\n"
            "Lawrence Ross,School D,MetLife Inc.,24\r\n";
        return data;
    }

    static const std::string& jsonData() {
        static std::string data =
            "{\t\"name\":\"Eleanor Little\",\n\t\"age\":43,\n\t\"company\":\"Conectiv, Inc\"}\n"
            "{\t\"name\":\"Rosie Hughes\",\n\t\"age\":44,\n\t\"company\":\"Western Gas Resources Inc\"}\n";
        return data;
    }

    static const std::string& jsonDataFull() {
        static std::string data =
            "{\n\t\"name\": \"Lora Francis\",\n\t\"age\": 27,\n\t\"company\": \"Staples Inc\"\n}\n"
            "{\n\t\"name\": \"Eleanor Little\",\n\t\"age\": 43,\n\t\"company\": \"Conectiv, Inc\"\n}\n"
            "{\n\t\"name\": \"Rosie Hughes\",\n\t\"age\": 44,\n\t\"company\": \"Western Gas Resources Inc\"\n}\n"
            "{\n\t\"name\": \"Lawrence Ross\",\n\t\"age\": 24,\n\t\"company\": \"MetLife Inc.\"\n}\n";
        return data;
    }
};

std::string ObjectSelectTest::bucketName_ = "";


TEST_F(ObjectSelectTest, SelectObject_Csv) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(csvData())));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Ignore")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                        .setCommentCharacter(base64Encode("#"))
                        .setAllowQuotedRecordDelimiter(true)
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setCsv(models::CSVOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                    )
                    .setOutputRawData(false)
                    .setKeepAllColumns(true)
                    .setEnablePayloadCrc(true)
                    .setOutputHeader(false)
                )
                .setOptions(models::SelectRequestOptions()
                    .setSkipPartialDataRecord(false)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "Lora Francis,School,Staples Inc,27\n"
        "Eleanor Little,School,\"Conectiv, Inc\",43\n"
        "Rosie Hughes,School,Western Gas Resources Inc,44\n"
        "Lawrence Ross,School,MetLife Inc.,24\n";
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_CsvWithSinkFactory) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-sink-factory";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(csvData())));
    ASSERT_TRUE(putOutcome.has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&)
            -> std::shared_ptr<ByteWriter> {
        userStream->str("");
        userStream->clear();
        return std::make_shared<OStreamWriter>(userStream);
    };

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSinkFactory(factory)
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Ignore")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                        .setCommentCharacter(base64Encode("#"))
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setCsv(models::CSVOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                    )
                    .setOutputRawData(false)
                    .setKeepAllColumns(true)
                    .setEnablePayloadCrc(true)
                    .setOutputHeader(false)
                )
                .setOptions(models::SelectRequestOptions()
                    .setSkipPartialDataRecord(false)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    std::string body = userStream->str();
    std::string expected =
        "Lora Francis,School,Staples Inc,27\n"
        "Eleanor Little,School,\"Conectiv, Inc\",43\n"
        "Rosie Hughes,School,Western Gas Resources Inc,44\n"
        "Lawrence Ross,School,MetLife Inc.,24\n";
    EXPECT_EQ(expected, body);

    EXPECT_EQ(nullptr, outcome.value().getBody());
}

TEST_F(ObjectSelectTest, SelectObject_CsvWithHeader) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-header";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(csvDataCRLF())));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select name from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setOutputHeader(true)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "name\n"
        "Lora Francis\n"
        "Eleanor Little\n"
        "Rosie Hughes\n"
        "Lawrence Ross\n";
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_CsvRaw) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-raw";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(csvDataCRLF())));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                        .setCommentCharacter(base64Encode("#"))
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setOutputRawData(true)
                    .setEnablePayloadCrc(false)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("true", it->second);

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "Lora Francis,School A,Staples Inc,27\r\n"
        "Eleanor Little,School B,\"Conectiv, Inc\",43\r\n"
        "Rosie Hughes,School C,Western Gas Resources Inc,44\r\n"
        "Lawrence Ross,School D,MetLife Inc.,24\r\n";
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_Json) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-json";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(jsonData())));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject as s where cast(s.age as int) > 43"))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                        .setParseJsonNumberAsString(true)
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setJson(models::JSONOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                    .setOutputRawData(false)
                    .setKeepAllColumns(true)
                    .setEnablePayloadCrc(true)
                    .setOutputHeader(false)
                )
                .setOptions(models::SelectRequestOptions()
                    .setSkipPartialDataRecord(false)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "{\"name\":\"Rosie Hughes\",\"age\":44,\"company\":\"Western Gas Resources Inc\"}\n";
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_JsonWithHeader) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-json-header";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(jsonDataFull())));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select name from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setOutputHeader(true)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "{\"name\":\"Lora Francis\"}\n"
        "{\"name\":\"Eleanor Little\"}\n"
        "{\"name\":\"Rosie Hughes\"}\n"
        "{\"name\":\"Lawrence Ross\"}\n";
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, CreateSelectObjectMeta_Csv) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-meta-csv";
    std::string data = "name,school,company,age\nLora Francis,School,Staples Inc,27\nEleanor Little,School,\"Conectiv, Inc\",43\nRosie Hughes,School,Western Gas Resources Inc,44\nLawrence Ross,School,MetLife Inc.,24";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("None")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(static_cast<int64_t>(data.size()), outcome.value().getTotalScanned());
    EXPECT_EQ(1, outcome.value().getSplitsCount());
    EXPECT_EQ(5, outcome.value().getRowsCount());
    EXPECT_EQ(4, outcome.value().getColumnsCount());
}

TEST_F(ObjectSelectTest, CreateSelectObjectMeta_Json) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-meta-json";
    std::string data =
        "{\n\t\"name\": \"Lora Francis\",\n\t\"age\": 27,\n\t\"company\": \"Staples Inc\"\n}\n"
        "{\n\t\"k2\": [-1, 79, 90],\n\t\"k3\": {\n\t\t\"k2\": 5,\n\t\t\"k3\": 1,\n\t\t\"k4\": 0\n\t}\n}\n"
        "{\n\t\"k1\": 1,\n\t\"k2\": {\n\t\t\"k2\": 5\n\t},\n\t\"k3\": []\n}";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/meta")
            .setSelectMetaRequest(models::JSONMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(static_cast<int64_t>(data.size()), outcome.value().getTotalScanned());
    EXPECT_EQ(1, outcome.value().getSplitsCount());
    EXPECT_EQ(3, outcome.value().getRowsCount());
    EXPECT_EQ(0, outcome.value().getColumnsCount());
}

TEST_F(ObjectSelectTest, SelectObject_SkipPartialData) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-skip-partial";
    std::string data = std::string(csvDataCRLF()) + "\r\n1,,3\r\n4,,6\r\n7,8,9\r\n";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome.has_value());

    // maxSkippedRecordsAllowed=1, expect failure
    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select _1,_2,_3,_4 from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                        .setCommentCharacter(base64Encode("#"))
                    )
                )
                .setOptions(models::SelectRequestOptions()
                    .setSkipPartialDataRecord(true)
                    .setMaxSkippedRecordsAllowed(1)
                )
            ));
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidCsvLine", outcome.error().getCode());

    // maxSkippedRecordsAllowed=5, expect success
    auto retryOutcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select _1,_2,_3,_4 from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteCharacter(base64Encode("\""))
                        .setCommentCharacter(base64Encode("#"))
                    )
                )
                .setOptions(models::SelectRequestOptions()
                    .setSkipPartialDataRecord(true)
                    .setMaxSkippedRecordsAllowed(5)
                )
            ));
    ASSERT_TRUE(retryOutcome.has_value());
    EXPECT_EQ(206, retryOutcome.value().getStatusCode());

    auto body = streamToString(retryOutcome.value().getBody());
    EXPECT_NE(std::string::npos, body.find("Lora Francis"));
    EXPECT_NE(std::string::npos, body.find("Lawrence Ross"));
}

TEST_F(ObjectSelectTest, SelectObject_GzipData) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "sample_data.csv.gz";
    std::string filePath = TEST_DATA_PATH "sample_data.csv.gz";

    std::ifstream file(filePath, std::ios::binary);
    ASSERT_TRUE(file.good()) << "Test data file not found: " << filePath;
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(content)));
    ASSERT_TRUE(putOutcome.has_value());

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCompressionType("GZIP")
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto body = streamToString(outcome.value().getBody());
    EXPECT_GT(body.size(), 1000u);
}

TEST_F(ObjectSelectTest, SelectObject_CsvRange) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-range";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(csvDataCRLF())));
    ASSERT_TRUE(putOutcome.has_value());

    // createSelectObjectMeta first (required for range queries)
    auto metaOutcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                    )
                )
            ));
    ASSERT_TRUE(metaOutcome.has_value());
    ASSERT_EQ(1, metaOutcome.value().getSplitsCount());
    ASSERT_EQ(5, metaOutcome.value().getRowsCount());
    ASSERT_EQ(4, metaOutcome.value().getColumnsCount());

    // line-range
    auto lineOutcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                        .setRange("line-range=1-3")
                    )
                )
            ));
    ASSERT_TRUE(lineOutcome.has_value());
    EXPECT_EQ(206, lineOutcome.value().getStatusCode());

    auto lineBody = streamToString(lineOutcome.value().getBody());
    std::string expectedAll =
        "Lora Francis,School A,Staples Inc,27\r\n"
        "Eleanor Little,School B,\"Conectiv, Inc\",43\r\n"
        "Rosie Hughes,School C,Western Gas Resources Inc,44\r\n";
    EXPECT_EQ(expectedAll, lineBody);

    // split-range
    auto splitOutcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\r\n"))
                        .setRange("split-range=0-1")
                    )
                )
            ));
    //std::cout << splitOutcome.error().toString() << std::endl;
    ASSERT_TRUE(splitOutcome.has_value());
    EXPECT_EQ(206, splitOutcome.value().getStatusCode());
    auto splitBody = streamToString(splitOutcome.value().getBody());

    expectedAll =
        "Lora Francis,School A,Staples Inc,27\r\n"
        "Eleanor Little,School B,\"Conectiv, Inc\",43\r\n"
        "Rosie Hughes,School C,Western Gas Resources Inc,44\r\n"
        "Lawrence Ross,School D,MetLife Inc.,24\r\n";
    EXPECT_EQ(expectedAll, splitBody);
}

TEST_F(ObjectSelectTest, CreateSelectObjectMeta_Delimiters) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-meta-delimiters";
    std::string data = "abc,def123,456|7891334\n777,888|999,222012345\n\n";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome.has_value());

    // field=, record=\n
    auto outcome1 = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFieldDelimiter(base64Encode(","))
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome1.has_value());
    EXPECT_EQ(200, outcome1.value().getStatusCode());
    EXPECT_EQ(1, outcome1.value().getSplitsCount());
    EXPECT_EQ(3, outcome1.value().getRowsCount());
    EXPECT_EQ(3, outcome1.value().getColumnsCount());

    // without overwrite (cached meta still used)
    auto outcome2 = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFieldDelimiter(base64Encode("|"))
                        .setRecordDelimiter(base64Encode("\n\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome2.has_value());
    EXPECT_EQ(200, outcome2.value().getStatusCode());
    EXPECT_EQ(1, outcome2.value().getSplitsCount());
    EXPECT_EQ(3, outcome2.value().getRowsCount());
    EXPECT_EQ(3, outcome2.value().getColumnsCount());

    // with overwrite
    auto outcome3 = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFieldDelimiter(base64Encode("|"))
                        .setRecordDelimiter(base64Encode("\n\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome3.has_value());
    EXPECT_EQ(200, outcome3.value().getStatusCode());
    EXPECT_EQ(1, outcome3.value().getSplitsCount());
    EXPECT_EQ(1, outcome3.value().getRowsCount());
    EXPECT_EQ(3, outcome3.value().getColumnsCount());
}

TEST_F(ObjectSelectTest, CreateSelectObjectMeta_QuoteChar) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-meta-quotechar";
    std::string data = "'abc','def\n123','456'\n";

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome.has_value());

    // quoteChar = '
    auto outcome1 = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setQuoteCharacter(base64Encode("'"))
                    )
                )
            ));
    ASSERT_TRUE(outcome1.has_value());
    EXPECT_EQ(200, outcome1.value().getStatusCode());
    EXPECT_EQ(1, outcome1.value().getSplitsCount());
    EXPECT_EQ(1, outcome1.value().getRowsCount());
    EXPECT_EQ(3, outcome1.value().getColumnsCount());

    // re-put and use quoteChar = "
    auto putOutcome2 = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(data)));
    ASSERT_TRUE(putOutcome2.has_value());

    auto outcome2 = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setQuoteCharacter(base64Encode("\""))
                    )
                )
            ));
    ASSERT_TRUE(outcome2.has_value());
    EXPECT_EQ(200, outcome2.value().getStatusCode());
    EXPECT_EQ(1, outcome2.value().getSplitsCount());
    EXPECT_EQ(2, outcome2.value().getRowsCount());
    EXPECT_EQ(2, outcome2.value().getColumnsCount());
}

static std::string readFileContent(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.good()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

static std::vector<std::string> splitLines(const std::string& content, const std::string& delim) {
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < content.size()) {
        auto next = content.find(delim, pos);
        if (next == std::string::npos) {
            auto remainder = content.substr(pos);
            if (!remainder.empty()) lines.push_back(remainder);
            break;
        }
        lines.push_back(content.substr(pos, next - pos));
        pos = next + delim.size();
    }
    return lines;
}

struct CsvRow {
    std::vector<std::string> fields;
    std::map<std::string, size_t> headerIndex;

    std::string get(const std::string& col) const {
        auto it = headerIndex.find(col);
        if (it == headerIndex.end() || it->second >= fields.size()) return "";
        return fields[it->second];
    }
};

static std::vector<CsvRow> parseCsvFile(const std::string& content, const std::string& lineDelim) {
    auto lines = splitLines(content, lineDelim);
    if (lines.empty()) return {};

    auto headerFields = splitCsvLine(lines[0]);
    std::map<std::string, size_t> headerIndex;
    for (size_t i = 0; i < headerFields.size(); i++) {
        headerIndex[headerFields[i]] = i;
    }

    std::vector<CsvRow> rows;
    for (size_t i = 1; i < lines.size(); i++) {
        if (lines[i].empty()) continue;
        CsvRow row;
        row.fields = splitCsvLine(lines[i]);
        row.headerIndex = headerIndex;
        rows.push_back(std::move(row));
    }
    return rows;
}

static bool endsWith(const std::string& s, const std::string& suffix) {
    if (suffix.size() > s.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static double toDouble(const std::string& s) {
    if (s.empty()) return 0.0;
    char* end = nullptr;
    double val = std::strtod(s.c_str(), &end);
    if (end == s.c_str()) return 0.0;
    return val;
}

static std::vector<std::string> splitJsonRecords(const std::string& body, const std::string& recordDelim) {
    std::vector<std::string> records;
    std::string trimmed = body;
    if (!trimmed.empty() && trimmed.back() == recordDelim[0]) {
        trimmed.pop_back();
    }
    std::string sep = recordDelim + "{";
    size_t pos = 0;
    while (pos < trimmed.size()) {
        auto next = trimmed.find(sep, pos);
        if (next == std::string::npos) {
            records.push_back(trimmed.substr(pos));
            break;
        }
        records.push_back(trimmed.substr(pos, next - pos));
        pos = next + recordDelim.size();
    }
    return records;
}

static std::string extractJsonStringField(const std::string& record, const std::string& field) {
    std::string pattern = "\"" + field + "\":\"";
    auto pos = record.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    auto end = record.find('"', pos);
    if (end == std::string::npos) return "";
    return record.substr(pos, end - pos);
}

TEST_F(ObjectSelectTest, SelectObject_CsvConcat) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-concat";
    std::string filePath = TEST_DATA_PATH "sample_data.csv";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty()) << "Test data file not found: " << filePath;

#ifdef _WIN32
    utils::StringReplace(fileContent, "\r\n", "\n");
#endif

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    std::string expression = "select Year,StateAbbr, CityName, Short_Question_Text from ossobject where (data_value || data_value_unit) = '14.8%'";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());

    auto rows = parseCsvFile(fileContent, "\n");
    std::string expected;
    std::string outputDelim = "\n";
    for (const auto& row : rows) {
        if (row.get("Data_Value_Unit") == "%" && row.get("Data_Value") == "14.8") {
            expected += row.get("Year") + "," + row.get("StateAbbr") + "," +
                        row.get("CityName") + "," + row.get("Short_Question_Text") + outputDelim;
        }
    }
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_CsvConcatWithOutputDelimiter) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-concat-delim";
    std::string filePath = TEST_DATA_PATH "sample_data.csv";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty());

#ifdef _WIN32
    utils::StringReplace(fileContent, "\r\n", "\n");
#endif

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    std::string expression = "select Year,StateAbbr, CityName, Short_Question_Text from ossobject where (data_value || data_value_unit) = '14.8%'";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setCsv(models::CSVOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto body = streamToString(outcome.value().getBody());

    auto rows = parseCsvFile(fileContent, "\n");
    std::string expected;
    for (const auto& row : rows) {
        if (row.get("Data_Value_Unit") == "%" && row.get("Data_Value") == "14.8") {
            expected += row.get("Year") + "," + row.get("StateAbbr") + "," +
                        row.get("CityName") + "," + row.get("Short_Question_Text") + "\n";
        }
    }
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_CsvComplexWhere) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-complex-where";
    std::string filePath = TEST_DATA_PATH "sample_data.csv";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty());

#ifdef _WIN32
    utils::StringReplace(fileContent, "\r\n", "\n");
#endif

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    std::string expression =
        "select Year,StateAbbr, CityName, Short_Question_Text, data_value, data_value_unit, "
        "category, high_confidence_limit from ossobject where data_value > 14.8 and "
        "data_value_unit = '%' or Measure like '%18 Years' and Category = 'Unhealthy Behaviors' "
        "or high_confidence_limit > 70.0 ";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());

    auto rows = parseCsvFile(fileContent, "\n");
    std::string expected;
    std::string outputDelim = "\n";
    for (const auto& row : rows) {
        auto dataValue = row.get("Data_Value");
        auto dataValueUnit = row.get("Data_Value_Unit");
        auto measure = row.get("Measure");
        auto category = row.get("Category");
        auto highConfLimit = row.get("High_Confidence_Limit");

        bool cond1 = !dataValue.empty() && toDouble(dataValue) > 14.8 && dataValueUnit == "%";
        bool cond2 = endsWith(measure, "18 Years") && category == "Unhealthy Behaviors";
        bool cond3 = !highConfLimit.empty() && toDouble(highConfLimit) > 70.0;

        if (cond1 || cond2 || cond3) {
            expected += row.get("Year") + "," + row.get("StateAbbr") + "," +
                        row.get("CityName") + "," + row.get("Short_Question_Text") + "," +
                        dataValue + "," + dataValueUnit + "," +
                        category + "," + highConfLimit + outputDelim;
        }
    }
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_CsvComplexWhereRaw) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-csv-complex-where-raw";
    std::string filePath = TEST_DATA_PATH "sample_data.csv";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty());

#ifdef _WIN32
    utils::StringReplace(fileContent, "\r\n", "\n");
#endif

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    std::string expression =
        "select Year,StateAbbr, CityName, Short_Question_Text, data_value, data_value_unit, "
        "category, high_confidence_limit from ossobject where data_value > 14.8 and "
        "data_value_unit = '%' or Measure like '%18 Years' and Category = 'Unhealthy Behaviors' "
        "or high_confidence_limit > 70.0 ";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setFileHeaderInfo("Use")
                        .setRecordDelimiter(base64Encode("\n"))
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setOutputRawData(true)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("true", it->second);

    auto body = streamToString(outcome.value().getBody());

    auto rows = parseCsvFile(fileContent, "\n");
    std::string expected;
    std::string outputDelim = "\n";
    for (const auto& row : rows) {
        auto dataValue = row.get("Data_Value");
        auto dataValueUnit = row.get("Data_Value_Unit");
        auto measure = row.get("Measure");
        auto category = row.get("Category");
        auto highConfLimit = row.get("High_Confidence_Limit");

        bool cond1 = !dataValue.empty() && toDouble(dataValue) > 14.8 && dataValueUnit == "%";
        bool cond2 = endsWith(measure, "18 Years") && category == "Unhealthy Behaviors";
        bool cond3 = !highConfLimit.empty() && toDouble(highConfLimit) > 70.0;

        if (cond1 || cond2 || cond3) {
            expected += row.get("Year") + "," + row.get("StateAbbr") + "," +
                        row.get("CityName") + "," + row.get("Short_Question_Text") + "," +
                        dataValue + "," + dataValueUnit + "," +
                        category + "," + highConfLimit + outputDelim;
        }
    }
    EXPECT_EQ(expected, body);
}

TEST_F(ObjectSelectTest, SelectObject_JsonLineRange) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-json-line-range";
    std::string filePath = TEST_DATA_PATH "sample_json_lines.json";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty()) << "Test data file not found: " << filePath;

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    auto metaOutcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/meta")
            .setSelectMetaRequest(models::JSONMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
            ));
    ASSERT_TRUE(metaOutcome.has_value());

    std::string expression = "select person.firstname as firstname, person.lastname, extra from ossobject";
    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                        .setRange("line-range=10-50")
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setJson(models::JSONOutputFormat()
                        .setRecordDelimiter(base64Encode(","))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());
    EXPECT_FALSE(body.empty());

    auto records = splitJsonRecords(body, ",");
    EXPECT_EQ(records.size(), 41u);

    for (const auto& record : records) {
        EXPECT_NE(std::string::npos, record.find("firstname"));
    }
}

TEST_F(ObjectSelectTest, SelectObject_JsonComplexWhere) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-json-complex-where";
    std::string filePath = TEST_DATA_PATH "sample_json_lines.json";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty());

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    auto metaOutcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/meta")
            .setSelectMetaRequest(models::JSONMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
            ));
    ASSERT_TRUE(metaOutcome.has_value());

    std::string expression =
        "select person.firstname, person.lastname, congress_numbers from ossobject "
        "where startdate > '2017-01-01' and senator_rank = 'junior' or state = 'CA' and party = 'Republican'";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setJson(models::JSONOutputFormat()
                        .setRecordDelimiter(base64Encode(","))
                    )
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("false", it->second);

    auto body = streamToString(outcome.value().getBody());
    EXPECT_FALSE(body.empty());

    auto records = splitJsonRecords(body, ",");
    EXPECT_EQ(20u, records.size());

    for (const auto& rec : records) {
        auto firstname = extractJsonStringField(rec, "firstname");
        auto lastname = extractJsonStringField(rec, "lastname");
        EXPECT_FALSE(firstname.empty());
        EXPECT_FALSE(lastname.empty());
        EXPECT_NE(std::string::npos, rec.find("congress_numbers"));
    }

    EXPECT_EQ("Roy", extractJsonStringField(records[0], "firstname"));
    EXPECT_EQ("Blunt", extractJsonStringField(records[0], "lastname"));
    EXPECT_EQ("Jerry", extractJsonStringField(records[1], "firstname"));
    EXPECT_EQ("Moran", extractJsonStringField(records[1], "lastname"));
    EXPECT_EQ("Robert", extractJsonStringField(records[2], "firstname"));
    EXPECT_EQ("Portman", extractJsonStringField(records[2], "lastname"));
}

TEST_F(ObjectSelectTest, SelectObject_JsonComplexWhereRaw) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "select-object-json-complex-where-raw";
    std::string filePath = TEST_DATA_PATH "sample_json_lines.json";
    std::string fileContent = readFileContent(filePath);
    ASSERT_FALSE(fileContent.empty());

    auto putOutcome = client->putObject(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::fromString(fileContent)));
    ASSERT_TRUE(putOutcome.has_value());

    auto metaOutcome = client->createSelectObjectMeta(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/meta")
            .setSelectMetaRequest(models::JSONMetaRequest()
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
            ));
    ASSERT_TRUE(metaOutcome.has_value());

    std::string expression =
        "select person.firstname, person.lastname, congress_numbers from ossobject "
        "where startdate > '2017-01-01' and senator_rank = 'junior' or state = 'CA' and party = 'Republican'";

    auto outcome = client->selectObject(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode(expression))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setType("LINES")
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setJson(models::JSONOutputFormat()
                        .setRecordDelimiter(base64Encode(","))
                    )
                    .setOutputRawData(true)
                )
            ));
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto& headers = outcome.value().getHeaders();
    auto it = headers.find("x-oss-select-output-raw");
    ASSERT_NE(headers.end(), it);
    EXPECT_EQ("true", it->second);

    auto body = streamToString(outcome.value().getBody());
    EXPECT_FALSE(body.empty());

    auto records = splitJsonRecords(body, ",");
    EXPECT_EQ(20u, records.size());

    EXPECT_EQ("Roy", extractJsonStringField(records[0], "firstname"));
    EXPECT_EQ("Blunt", extractJsonStringField(records[0], "lastname"));
    EXPECT_EQ("Jerry", extractJsonStringField(records[1], "firstname"));
    EXPECT_EQ("Moran", extractJsonStringField(records[1], "lastname"));
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
