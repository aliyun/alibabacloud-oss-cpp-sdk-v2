#include <gtest/gtest.h>

#include "TestUtils.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/Error.h"

#include <fstream>

namespace alibabacloud::oss2 {

TEST(OperationTest, OperationOptions_Constructor) {
    auto opt = OperationOptions();
    EXPECT_FALSE(opt.retryMaxAttempts.has_value());
    EXPECT_FALSE(opt.readWriteTimeout.has_value());
    EXPECT_FALSE(opt.cancellationToken.has_value());

    auto opt1 = OperationOptions{10, 20};
    EXPECT_EQ(10, opt1.retryMaxAttempts);
    EXPECT_EQ(20, opt1.readWriteTimeout);
    EXPECT_FALSE(opt1.cancellationToken.has_value());
}


TEST(OperationTest, OperationInput_Constructor) {
    auto input = OperationInput();
    EXPECT_EQ("", input.opName);
    EXPECT_EQ("", input.method);
    EXPECT_EQ(0, input.headers.size());
    EXPECT_EQ(0, input.parameters.size());
    EXPECT_EQ(false, input.bucket.has_value());
    EXPECT_EQ(false, input.key.has_value());
    EXPECT_EQ(0, input.opMetadata.size());
    EXPECT_EQ(nullptr, input.body);

    auto input1 = OperationInput{"OP", "GET"};
    EXPECT_EQ("OP", input1.opName);
    EXPECT_EQ("GET", input1.method);
    EXPECT_EQ(0, input1.headers.size());
    EXPECT_EQ(0, input1.parameters.size());
    EXPECT_EQ(false, input1.bucket.has_value());
    EXPECT_EQ(false, input1.key.has_value());
    EXPECT_EQ(0, input1.opMetadata.size());
    EXPECT_EQ(nullptr, input1.body);
}

TEST(OperationTest, RequestBodyFromString) {
    std::string data = "hello world";
    const std::string constdata = data;
    std::string got;
    got.resize(32);

    // lvalue string
    auto body = RequestBody::FromString(data);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    auto source = body->spanSource();
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // rvalue string
    body = RequestBody::FromString(std::string("hello world"));
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // const string
    body = RequestBody::FromString(constdata);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(data.substr(0, n), got.substr(0, n));
}


TEST(OperationTest, RequestBodyFromStream) {
    std::string data = "hello world";
    std::string got;
    got.resize(32);
    auto stream = std::make_shared<std::istringstream>(data);

    // lvalue string
    auto body = RequestBody::FromStream(stream);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    auto source = body->spanSource();
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // rvalue string
    body = RequestBody::FromStream(std::make_shared<std::istringstream>(data));
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // const string
    const auto conststream = std::make_shared<std::istringstream>(data);
    body = RequestBody::FromStream(conststream);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(data.substr(0, n), got.substr(0, n));

    // nullptr
    body = RequestBody::FromStream(nullptr);
    EXPECT_EQ(0, body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 32);
    EXPECT_EQ(0, n);
}

TEST(OperationTest, RequestBodyFromFilePathString) {
    std::string data = "hello world";
    auto len = data.length();
    auto filepath = TestUtils::GenRandomFile(data.size());
    auto fileData = TestUtils::GetFileContent(filepath);
    std::string got;
    got.resize(32);

    // File
    EXPECT_EQ(len, fileData.size());

    // lvalue string
    auto body = RequestBody::FromFile(filepath);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    auto source = body->spanSource();
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(fileData, got.substr(0, data.length()));

    // rvalue string
    auto rfilepath = filepath;
    body = RequestBody::FromFile(std::move(rfilepath));
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(fileData, got.substr(0, data.length()));

    // const string
    const std::string constfilepath = filepath;
    body = RequestBody::FromFile(constfilepath);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(fileData.substr(0, n), got.substr(0, n));
}


TEST(OperationTest, RequestBodyFromFilePath) {
    std::string data = "hello world";
    auto len = data.length();
    auto filepath = TestUtils::GenRandomFile(data.size());
    auto fileData = TestUtils::GetFileContent(filepath);
    std::string got;
    got.resize(32);

    // File
    EXPECT_EQ(len, fileData.size());

    // lvalue string
    auto path = std::filesystem::path(filepath);
    auto body = RequestBody::FromFile(path);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    auto source = body->spanSource();
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(fileData, got.substr(0, data.length()));

    // rvalue string
    body = RequestBody::FromFile(std::filesystem::path(filepath));
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(fileData, got.substr(0, data.length()));

    // const string
    const auto constpath = std::filesystem::path(filepath);
    body = RequestBody::FromFile(constpath);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(filepath, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(fileData.substr(0, n), got.substr(0, n));
}


TEST(OperationTest, RequestBodyFromMemory) {
    std::string data = "hello world";
    std::string_view dataview = data;
    const std::string constdata = data;
    std::string got;
    got.resize(32);

    // lvalue std::string_view
    auto body = RequestBody::FromMemory(dataview);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    auto source = body->spanSource();
    auto n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // rvalue std::string_view
    std::string_view rdataview = data;
    body = RequestBody::FromMemory(std::move(rdataview));
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), data.length());
    EXPECT_EQ(data.length(), n);
    EXPECT_EQ(data, got.substr(0, data.length()));

    // const string
    const std::string_view cdataview = data;
    body = RequestBody::FromMemory(cdataview);
    EXPECT_EQ(data.length(), body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(data.substr(0, n), got.substr(0, n));

    // const char* non-nullptr
    auto len = 3;
    body = RequestBody::FromMemory(data.c_str(), len);
    EXPECT_EQ(len, body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->read(reinterpret_cast<uint8_t*>(got.data()), 11);
    EXPECT_EQ(3, n);
    EXPECT_EQ(data.substr(0, 3), got.substr(0, n));

    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 32);
    EXPECT_EQ(0, n);

    // const char* nullptr
    body = RequestBody::FromMemory(nullptr, 11);
    EXPECT_EQ(0, body->length().value());
    EXPECT_FALSE(body->isOneShot());
    EXPECT_EQ(std::nullopt, body->path());

    source = body->spanSource();
    n = source->readToCount(reinterpret_cast<uint8_t*>(got.data()), 32);
    EXPECT_EQ(0, n);
}


TEST(OperationTest, OperationMetadata) {
    auto input = OperationInput();
    input.opMetadata.emplace("ARRAY-LIST", std::vector<std::string>{"key1", "key2"});
    input.opMetadata.emplace("BOOL", true);

    EXPECT_TRUE(input.opMetadata.find("ARRAY-LIST") != input.opMetadata.end());
    auto& value = input.opMetadata.at("ARRAY-LIST");
    if (auto val = std::get_if<std::vector<std::string>>(&value)) {
        std::cout << "ARRAY-LIST:";
        for (const auto& it : *val) {
            std::cout << it;
        }
        std::cout << std::endl;
    }

    EXPECT_TRUE(input.opMetadata.find("BOOL") != input.opMetadata.end());
    value = input.opMetadata.at("BOOL");
    if (auto val = std::get_if<bool>(&value)) {
        std::cout << "BOOL:";
        std::cout << *val << std::endl;
    }

    EXPECT_FALSE(input.opMetadata.find("not-exist") != input.opMetadata.end());
}

TEST(OperationTest, ToString_ServerError) {
    std::map<std::string, std::string> fields{
        {"Code", "NoSuchKey"},
        {"Message", "The specified key does not exist."},
        {"RequestId", "5C3D8D2A0ACA54D87B43****"},
        {"EC", "0026-00000001"},
    };
    auto ec = make_server_error_code(404);
    OperationError err("GetObject", "GET", "https://bucket.oss-cn-hangzhou.aliyuncs.com/key", ec, fields);

    HeaderCollection headers;
    headers["Date"] = "Mon, 19 May 2026 12:00:00 GMT";
    headers["x-oss-request-id"] = "5C3D8D2A0ACA54D87B43****";
    err.setResponseResult(404, std::move(headers), "");

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Service."));
    EXPECT_NE(std::string::npos, str.find("Http Status Code: 404"));
    EXPECT_NE(std::string::npos, str.find("Error Code: NoSuchKey"));
    EXPECT_NE(std::string::npos, str.find("Request Id: 5C3D8D2A0ACA54D87B43****"));
    EXPECT_NE(std::string::npos, str.find("Message: The specified key does not exist."));
    EXPECT_NE(std::string::npos, str.find("EC: 0026-00000001"));
    EXPECT_NE(std::string::npos, str.find("Mon, 19 May 2026 12:00:00 GMT"));
    EXPECT_NE(std::string::npos, str.find("Request Endpoint: https://bucket.oss-cn-hangzhou.aliyuncs.com/key"));
    EXPECT_EQ(std::string::npos, str.find("Error Category:"));
}

TEST(OperationTest, ToString_ServerError_RetryableCode) {
    std::map<std::string, std::string> fields{
        {"Code", "InternalError"},
        {"Message", "Please try again."},
    };
    auto ec = make_retryable_server_error_code(503);
    OperationError err("PutObject", "PUT", "https://bucket.oss-cn-hangzhou.aliyuncs.com/key", ec, fields);
    err.setResponseResult(503, {}, "");

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Service."));
    EXPECT_NE(std::string::npos, str.find("Http Status Code: 503"));
    EXPECT_NE(std::string::npos, str.find("Error Code: InternalError"));
}

TEST(OperationTest, ToString_ClientError) {
    std::map<std::string, std::string> fields{
        {"Code", "ArgumentInvalid"},
        {"Message", "bucket name is invalid"},
    };
    auto ec = make_error_code(ClientErrorCode::BucketNameInvalid);
    OperationError err("PutObject", "PUT", "", ec, fields);

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Client."));
    EXPECT_NE(std::string::npos, str.find("Error Category: oss2.client"));
    EXPECT_NE(std::string::npos, str.find("Error Code: ArgumentInvalid"));
    EXPECT_NE(std::string::npos, str.find("Message: bucket name is invalid"));
    EXPECT_NE(std::string::npos, str.find("Error Description:"));
    EXPECT_EQ(std::string::npos, str.find("Error returned by Service."));
    EXPECT_EQ(std::string::npos, str.find("Http Status Code:"));
}

TEST(OperationTest, ToString_TransportError) {
    std::map<std::string, std::string> fields{
        {"Code", "ConnectionFailed"},
        {"Message", "could not resolve host"},
    };
    auto ec = make_error_code(TransportErrorCode::DnsError);
    OperationError err("GetObject", "GET", "https://bucket.oss-cn-hangzhou.aliyuncs.com/key", ec, fields);

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Client."));
    EXPECT_NE(std::string::npos, str.find("Error Category: oss2.transport"));
    EXPECT_NE(std::string::npos, str.find("Error Code: ConnectionFailed"));
    EXPECT_NE(std::string::npos, str.find("Message: could not resolve host"));
    EXPECT_EQ(std::string::npos, str.find("Error returned by Service."));
}

TEST(OperationTest, ToString_CredentialsError) {
    std::map<std::string, std::string> fields{
        {"Code", "CredentialsEmpty"},
        {"Message", "credentials provider returned empty credentials"},
    };
    auto ec = make_error_code(CredentialsErrorCode::Empty);
    OperationError err("PutObject", "PUT", "", ec, fields);

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Client."));
    EXPECT_NE(std::string::npos, str.find("Error Category: oss2.credentials"));
    EXPECT_NE(std::string::npos, str.find("Error Code: CredentialsEmpty"));
    EXPECT_EQ(std::string::npos, str.find("Error returned by Service."));
}

TEST(OperationTest, ToString_SignerError) {
    std::map<std::string, std::string> fields{
        {"Code", "SignFailed"},
        {"Message", "signing failed"},
    };
    auto ec = make_error_code(SignerErrorCode::SignFailed);
    OperationError err("PutObject", "PUT", "", ec, fields);

    auto str = err.toString();
    EXPECT_NE(std::string::npos, str.find("Error returned by Client."));
    EXPECT_NE(std::string::npos, str.find("Error Category: oss2.signer"));
    EXPECT_EQ(std::string::npos, str.find("Error returned by Service."));
}

} // namespace alibabacloud::oss2
