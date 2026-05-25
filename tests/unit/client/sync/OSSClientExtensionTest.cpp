#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace alibabacloud::oss2 {

namespace {

class MockTransport : public HttpTransport {
  public:
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        if (request->body != nullptr) {
            auto src = request->body->spanSource();
            src->readToEnd();
        }
        requests.emplace_back(std::make_unique<RequestMessage>(*request));

        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return TransportError{std::make_error_code(std::errc::result_out_of_range)};
    }
    std::string getName() const override { return "MockTransport"; }

    std::vector<ResponseResult> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
};

class WritingMockTransport : public HttpTransport {
  public:
    struct Response {
        int statusCode{200};
        HeaderCollection headers;
        std::string body;
    };
    std::vector<Response> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;

    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        if (request->body != nullptr) {
            auto src = request->body->spanSource();
            src->readToEnd();
        }
        requests.emplace_back(std::make_unique<RequestMessage>(*request));

        auto r = std::move(responses.front());
        responses.erase(responses.begin());

        auto response = std::make_unique<ResponseMessage>();
        response->statusCode = r.statusCode;
        response->headers = r.headers;

        bool isError = (r.statusCode / 100 != 2) || (r.statusCode == 203);

        if (!isError && options.sinkFactory.has_value()) {
            auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(r.body.size()));
            if (sink) {
                auto* data = reinterpret_cast<const std::uint8_t*>(r.body.data());
                sink->write(data, r.body.size());
                if (sink->bad()) {
                    return TransportError{make_error_code(TransportErrorCode::SendRecvError),
                                          "WriteStreamError", "Failed to write response body"};
                }
            }
        } else {
            response->body = std::make_shared<std::stringstream>(r.body);
        }

        return response;
    }

    std::string getName() const override { return "WritingMockTransport"; }
};

OSSClient makeClient(std::shared_ptr<HttpTransport> transport) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = transport;
    return OSSClient(config);
}

std::string createTempFile(const std::string& content) {
    auto name = std::tmpnam(nullptr);
    std::ofstream f(name, std::ios::binary);
    f << content;
    f.close();
    return name;
}

} // namespace

// --- putObjectFromFile ---

TEST(OSSClientExtensionTest, PutObjectFromFile_Success) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string content = "hello from file upload";
    auto filePath = createTempFile(content);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}, {"x-oss-hash-crc64ecma", "12345"}}, nullptr}));

    auto outcome = client.putObjectFromFile(
            models::PutObjectRequest().setBucket("bucket").setKey("key"),
            filePath);

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, mock->requests.size());
    std::remove(filePath.c_str());
}


// --- isObjectExist ---

TEST(OSSClientExtensionTest, IsObjectExist_True) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}}, nullptr}));

    auto outcome = client.isObjectExist("bucket", "key");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST(OSSClientExtensionTest, IsObjectExist_False_NoSuchKey) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string errorBody = R"(<Error><Code>NoSuchKey</Code><Message>The specified key does not exist.</Message><RequestId>id-456</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404, "Not Found",
                    {{"x-oss-request-id", "id-456"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto outcome = client.isObjectExist("bucket", "no-such-key");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST(OSSClientExtensionTest, IsObjectExist_False_BadErrorResponse) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404, "Not Found",
                    {{"x-oss-request-id", "id-789"}},
                    std::make_shared<std::stringstream>("")}));

    auto outcome = client.isObjectExist("bucket", "key");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST(OSSClientExtensionTest, IsObjectExist_Error_Propagated) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string errorBody = R"(<Error><Code>InvalidAccessKeyId</Code><Message>Invalid key</Message><RequestId>id-err</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{403, "Forbidden",
                    {{"x-oss-request-id", "id-err"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto outcome = client.isObjectExist("bucket", "key");
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("InvalidAccessKeyId", outcome.error().getCode());
}

// --- isBucketExist ---

TEST(OSSClientExtensionTest, IsBucketExist_True) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string aclBody = R"(<?xml version="1.0" encoding="UTF-8"?><AccessControlPolicy><Owner><ID>123</ID></Owner><AccessControlList><Grant>private</Grant></AccessControlList></AccessControlPolicy>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}},
                    std::make_shared<std::stringstream>(aclBody)}));

    auto outcome = client.isBucketExist("bucket");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST(OSSClientExtensionTest, IsBucketExist_False_NoSuchBucket) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string errorBody = R"(<Error><Code>NoSuchBucket</Code><Message>The specified bucket does not exist.</Message><RequestId>id-456</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404, "Not Found",
                    {{"x-oss-request-id", "id-456"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto outcome = client.isBucketExist("no-such-bucket");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

TEST(OSSClientExtensionTest, IsBucketExist_True_NoPermission) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    std::string errorBody = R"(<Error><Code>AccessDenied</Code><Message>Access denied</Message><RequestId>id-789</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{403, "Forbidden",
                    {{"x-oss-request-id", "id-789"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto outcome = client.isBucketExist("bucket");
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST(OSSClientExtensionTest, IsBucketExist_Error_TransportFailure) {
    auto mock = std::make_shared<MockTransport>();
    auto client = makeClient(mock);

    mock->responses.emplace_back(TransportError{
            std::make_error_code(std::errc::connection_refused),
            "ConnectError", "Connection refused"});

    auto outcome = client.isBucketExist("bucket");
    EXPECT_FALSE(outcome.has_value());
}

// --- getObjectToFile ---

TEST(OSSClientExtensionTest, GetObjectToFile_Success) {
    auto mock = std::make_shared<WritingMockTransport>();
    auto client = makeClient(mock);

    std::string content = "hello download to file";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->responses.push_back({200, {{"x-oss-request-id", "id-123"},
                                     {"Content-Length", std::to_string(content.size())},
                                     {"x-oss-hash-crc64ecma", std::to_string(crc)}},
                               content});

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath);

    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSClientExtensionTest, GetObjectToFile_CRC64Mismatch) {
    auto mock = std::make_shared<WritingMockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    ClientOptionsFns fns = {[](ClientOptions& opt) {
        opt.featureFlags |= static_cast<int>(FeatureFlagsType::EnableCRC64CheckDownload);
    }};

    auto client = OSSClient(config, fns);

    std::string content = "data for crc mismatch test";
    mock->responses.push_back({200, {{"x-oss-request-id", "id-123"},
                                     {"Content-Length", std::to_string(content.size())},
                                     {"x-oss-hash-crc64ecma", "99999"}},
                               content});

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CrcMismatch", outcome.error().getCode());
    std::remove(filePath.c_str());
}

TEST(OSSClientExtensionTest, GetObjectToFile_CRC64Disabled) {
    auto mock = std::make_shared<WritingMockTransport>();
    auto client = makeClient(mock);

    std::string content = "data with wrong crc but disabled";
    mock->responses.push_back({200, {{"x-oss-request-id", "id-123"},
                                     {"Content-Length", std::to_string(content.size())},
                                     {"x-oss-hash-crc64ecma", "99999"}},
                               content});

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath);

    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSClientExtensionTest, GetObjectToFile_CRC64SkippedForRange) {
    auto mock = std::make_shared<WritingMockTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    ClientOptionsFns fns = {[](ClientOptions& opt) {
        opt.featureFlags |= static_cast<int>(FeatureFlagsType::EnableCRC64CheckDownload);
    }};

    auto client = OSSClient(config, fns);

    std::string content = "partial data";
    mock->responses.push_back({206, {{"x-oss-request-id", "id-123"},
                                     {"Content-Length", std::to_string(content.size())},
                                     {"x-oss-hash-crc64ecma", "99999"}},
                               content});

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("key").setRange("bytes=0-11"),
            filePath);

    EXPECT_TRUE(outcome.has_value());
    std::remove(filePath.c_str());
}

TEST(OSSClientExtensionTest, GetObjectToFile_ErrorResponse) {
    auto mock = std::make_shared<WritingMockTransport>();
    auto client = makeClient(mock);

    std::string errorBody = R"(<Error><Code>NoSuchKey</Code><Message>Not found</Message><RequestId>id-err</RequestId></Error>)";
    mock->responses.push_back({404, {{"x-oss-request-id", "id-err"}}, errorBody});

    auto filePath = std::string(std::tmpnam(nullptr));

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("no-key"),
            filePath);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    std::remove(filePath.c_str());
}

TEST(OSSClientExtensionTest, GetObjectToFile_TruncatesExistingFile) {
    auto mock = std::make_shared<WritingMockTransport>();
    auto client = makeClient(mock);

    auto filePath = std::string(std::tmpnam(nullptr));

    {
        std::ofstream f(filePath, std::ios::binary);
        f << std::string(1024, 'X');
    }

    std::string content = "short";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());
    mock->responses.push_back({200, {{"x-oss-request-id", "id-123"},
                                     {"Content-Length", std::to_string(content.size())},
                                     {"x-oss-hash-crc64ecma", std::to_string(crc)}},
                               content});

    auto outcome = client.getObjectToFile(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath);

    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

} // namespace alibabacloud::oss2
