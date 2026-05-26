#include <gtest/gtest.h>

#include "Config.h"
#include "async/ClientHelper.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace async {

using utils::base64Encode;

static std::string streamToString(const std::shared_ptr<std::iostream>& s) {
    if (!s) return "";
    std::ostringstream ss;
    ss << s->rdbuf();
    return ss.str();
}

class AsyncObjectSelectTest : public ::testing::Test {
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

    static const std::string& jsonData() {
        static std::string data =
            "{\t\"name\":\"Eleanor Little\",\n\t\"age\":43,\n\t\"company\":\"Conectiv, Inc\"}\n"
            "{\t\"name\":\"Rosie Hughes\",\n\t\"age\":44,\n\t\"company\":\"Western Gas Resources Inc\"}\n";
        return data;
    }
};

std::string AsyncObjectSelectTest::bucketName_ = "";


TEST_F(AsyncObjectSelectTest, SelectObject_Csv) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-select-object-csv";

    auto putFuture = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::FromString(csvData())));
    ASSERT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setHeaderInfo("Ignore")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteChar(base64Encode("\""))
                        .setCommentChar(base64Encode("#"))
                        .setAllowQuotedRecordDelimiter(true)
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setCsv(models::CSVOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteChar(base64Encode("\""))
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
    auto outcome = future.get();
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

TEST_F(AsyncObjectSelectTest, SelectObject_Json) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-select-object-json";

    auto putFuture = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::FromString(jsonData())));
    ASSERT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("json/select")
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject as s where cast(s.age as int) > 43"))
                .setInputSerialization(models::InputSerialization()
                    .setJson(models::JSONInputFormat()
                        .setJsonType("LINES")
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
    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(206, outcome.value().getStatusCode());

    auto body = streamToString(outcome.value().getBody());
    std::string expected =
        "{\"name\":\"Rosie Hughes\",\"age\":44,\"company\":\"Western Gas Resources Inc\"}\n";
    EXPECT_EQ(expected, body);
}

TEST_F(AsyncObjectSelectTest, SelectObject_WithSinkFactory) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-select-object-sink-factory";

    auto putFuture = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::FromString(csvData())));
    ASSERT_TRUE(putFuture.get().has_value());

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&)
            -> std::shared_ptr<ByteWriter> {
        userStream->str("");
        userStream->clear();
        return std::make_shared<OStreamWriter>(userStream);
    };

    auto future = client->asyncCall(
        models::SelectObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/select")
            .setSinkFactory(factory)
            .setSelectRequest(models::SelectRequest()
                .setExpression(base64Encode("select * from ossobject"))
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setHeaderInfo("Ignore")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteChar(base64Encode("\""))
                        .setCommentChar(base64Encode("#"))
                    )
                )
                .setOutputSerialization(models::OutputSerialization()
                    .setCsv(models::CSVOutputFormat()
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteChar(base64Encode("\""))
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
    auto outcome = future.get();
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

TEST_F(AsyncObjectSelectTest, CreateSelectObjectMeta_Csv) {
    auto client = ClientHelper::GetDefaultClient();
    std::string key = "async-select-object-meta-csv";
    std::string data = "name,school,company,age\nLora Francis,School,Staples Inc,27\nEleanor Little,School,\"Conectiv, Inc\",43\nRosie Hughes,School,Western Gas Resources Inc,44\nLawrence Ross,School,MetLife Inc.,24";

    auto putFuture = client->asyncCall(
        models::PutObjectRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setBody(RequestBody::FromString(data)));
    ASSERT_TRUE(putFuture.get().has_value());

    auto future = client->asyncCall(
        models::CreateSelectObjectMetaRequest()
            .setBucket(bucketName_)
            .setKey(key)
            .setProcess("csv/meta")
            .setSelectMetaRequest(models::CSVMetaRequest()
                .setOverwriteIfExists(true)
                .setInputSerialization(models::InputSerialization()
                    .setCsv(models::CSVInputFormat()
                        .setHeaderInfo("None")
                        .setRecordDelimiter(base64Encode("\n"))
                        .setFieldDelimiter(base64Encode(","))
                        .setQuoteChar(base64Encode("\""))
                    )
                )
            ));
    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(static_cast<int64_t>(data.size()), outcome.value().getTotalScanned());
    EXPECT_EQ(1, outcome.value().getSplitsCount());
    EXPECT_EQ(5, outcome.value().getRowsCount());
    EXPECT_EQ(4, outcome.value().getColumnsCount());
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
