#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>

namespace alibabacloud::oss2 {

namespace {

class MockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options,
                   RequestCallback callback) override {
        if (request->body != nullptr) {
            auto src = request->body->spanSource();
            src->readToEnd();
        }

        ResponseResult result;
        if (!responses.empty()) {
            result = std::move(responses.front());
            responses.erase(responses.begin());
        } else {
            result = TransportError{std::make_error_code(std::errc::result_out_of_range)};
        }

        if (std::holds_alternative<std::unique_ptr<ResponseMessage>>(result)) {
            auto& resp = std::get<std::unique_ptr<ResponseMessage>>(result);
            bool isError = (resp->statusCode / 100 != 2) || (resp->statusCode == 203);
            if (!isError && options.sinkFactory.has_value() && bodyData.has_value()) {
                auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(bodyData->size()));
                if (sink) {
                    auto* data = reinterpret_cast<const std::uint8_t*>(bodyData->data());
                    sink->write(data, bodyData->size());
                }
                bodyData.reset();
            }
        }

        callback(std::move(result), std::move(request));
    }

    std::string getName() const override { return "MockAsyncTransport"; }

    std::vector<ResponseResult> responses;
    std::optional<std::string> bodyData;
};

class RetryMockAsyncTransport : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options,
                   RequestCallback callback) override {
        callCount_++;
        if (callCount_ <= failCount_) {
            if (options.sinkFactory.has_value() && !partialData_.empty()) {
                auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(fullData_.size()));
                if (sink) {
                    auto* data = reinterpret_cast<const std::uint8_t*>(partialData_.data());
                    sink->write(data, partialData_.size());
                }
            }
            callback(TransportError{make_error_code(TransportErrorCode::ConnectionFailed)},
                     std::move(request));
            return;
        }

        if (options.sinkFactory.has_value() && !fullData_.empty()) {
            auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(fullData_.size()));
            if (sink) {
                auto* data = reinterpret_cast<const std::uint8_t*>(fullData_.data());
                sink->write(data, fullData_.size());
            }
        }

        auto response = std::make_unique<ResponseMessage>();
        response->statusCode = 200;
        response->headers = successHeaders_;
        callback(std::move(response), std::move(request));
    }

    std::string getName() const override { return "RetryMockAsyncTransport"; }

    int failCount_{1};
    std::string partialData_;
    std::string fullData_;
    HeaderCollection successHeaders_;

  private:
    int callCount_{0};
};

OSSAsyncClient makeAsyncClient(std::shared_ptr<AsyncHttpTransport> transport) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = transport;
    return OSSAsyncClient(config);
}

template<typename T>
struct AsyncResult {
    std::mutex mu;
    std::condition_variable cv;
    std::optional<T> result;

    void set(T val) {
        std::lock_guard<std::mutex> lock(mu);
        result = std::move(val);
        cv.notify_one();
    }

    T wait() {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [this]{ return result.has_value(); });
        return std::move(*result);
    }
};

} // namespace

// --- putObjectFromFileAsync ---

TEST(OSSAsyncClientExtensionTest, PutObjectFromFileAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string content = "async file upload";
    auto filePath = std::string(std::tmpnam(nullptr));
    {
        std::ofstream f(filePath, std::ios::binary);
        f << content;
    }

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}}, nullptr}));

    auto ar = std::make_shared<AsyncResult<PutObjectOutcome>>();
    client.putObjectFromFileAsync(
            models::PutObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](PutObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    std::remove(filePath.c_str());
}

// --- isObjectExistAsync ---

TEST(OSSAsyncClientExtensionTest, IsObjectExistAsync_True) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}}, nullptr}));

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client.isObjectExistAsync("bucket", "key",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST(OSSAsyncClientExtensionTest, IsObjectExistAsync_False) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string errorBody = R"(<Error><Code>NoSuchKey</Code><Message>Not found</Message><RequestId>id-456</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404, "Not Found",
                    {{"x-oss-request-id", "id-456"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client.isObjectExistAsync("bucket", "no-key",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

// --- isBucketExistAsync ---

TEST(OSSAsyncClientExtensionTest, IsBucketExistAsync_True) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string aclBody = R"(<?xml version="1.0" encoding="UTF-8"?><AccessControlPolicy><Owner><ID>123</ID></Owner><AccessControlList><Grant>private</Grant></AccessControlList></AccessControlPolicy>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"}},
                    std::make_shared<std::stringstream>(aclBody)}));

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client.isBucketExistAsync("bucket",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_TRUE(outcome.value());
}

TEST(OSSAsyncClientExtensionTest, IsBucketExistAsync_False) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string errorBody = R"(<Error><Code>NoSuchBucket</Code><Message>Not exist</Message><RequestId>id-456</RequestId></Error>)";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{404, "Not Found",
                    {{"x-oss-request-id", "id-456"}},
                    std::make_shared<std::stringstream>(errorBody)}));

    auto ar = std::make_shared<AsyncResult<BoolOutcome>>();
    client.isBucketExistAsync("no-bucket",
            [ar](BoolOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_FALSE(outcome.value());
}

// --- getObjectToFileAsync ---

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_Success) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string content = "async download content";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", std::to_string(crc)}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_CRC64ResetOnRetry) {
    auto mock = std::make_shared<RetryMockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    auto client = OSSAsyncClient(config);

    std::string content = "full download content after retry";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->failCount_ = 1;
    mock->partialData_ = "partial";
    mock->fullData_ = content;
    mock->successHeaders_ = {{"x-oss-request-id", "id-retry"},
                             {"Content-Length", std::to_string(content.size())},
                             {"x-oss-hash-crc64ecma", std::to_string(crc)}};

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_CRC64Mismatch) {
    auto mock = std::make_shared<MockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    auto client = OSSAsyncClient(config);

    std::string content = "data for async crc mismatch test";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", "99999"}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("CRCInconsistent", outcome.error().getCode());
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_CRC64Disabled) {
    auto mock = std::make_shared<MockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    config.disableDownloadCRC64Check = true;

    auto client = OSSAsyncClient(config);

    std::string content = "data with wrong crc but disabled";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", "99999"}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_CRC64SkippedForRange) {
    auto mock = std::make_shared<MockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    auto client = OSSAsyncClient(config);

    std::string content = "partial data";
    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{206, "Partial Content",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", "99999"}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key").setRange("bytes=0-11"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    std::remove(filePath.c_str());
}

// --- getObjectToFileAsync progress callback ---

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_WithProgressCallback) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string content = "async progress callback test data";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", std::to_string(crc)}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto records = std::make_shared<std::vector<std::tuple<std::size_t, std::size_t, std::int64_t>>>();
    ProgressCallback cb;
    cb.callback = [records](std::size_t increment, std::size_t transferred,
                            std::int64_t total, std::uintptr_t) {
        records->emplace_back(increment, transferred, total);
    };

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key").setProgressCallback(cb),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    ASSERT_FALSE(records->empty());
    auto& last = records->back();
    EXPECT_EQ(content.size(), std::get<1>(last));
    EXPECT_EQ(static_cast<std::int64_t>(content.size()), std::get<2>(last));
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_WithProgressCallbackAndCRC) {
    auto mock = std::make_shared<MockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    auto client = OSSAsyncClient(config);

    std::string content = "async progress and crc together";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", std::to_string(crc)}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto totalTransferred = std::make_shared<std::size_t>(0);
    ProgressCallback cb;
    cb.callback = [totalTransferred](std::size_t, std::size_t transferred,
                                     std::int64_t, std::uintptr_t) {
        *totalTransferred = transferred;
    };

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key").setProgressCallback(cb),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(content.size(), *totalTransferred);

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

TEST(OSSAsyncClientExtensionTest, GetObjectToFileAsync_NoProgressCallbackStillWorks) {
    auto mock = std::make_shared<MockAsyncTransport>();
    auto client = makeAsyncClient(mock);

    std::string content = "no progress callback async";
    uint64_t crc = utils::CalcCRC64(0, content.data(), content.size());

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                    {{"x-oss-request-id", "id-123"},
                     {"Content-Length", std::to_string(content.size())},
                     {"x-oss-hash-crc64ecma", std::to_string(crc)}}, nullptr}));
    mock->bodyData = content;

    auto filePath = std::string(std::tmpnam(nullptr));

    auto ar = std::make_shared<AsyncResult<GetObjectOutcome>>();
    client.getObjectToFileAsync(
            models::GetObjectRequest().setBucket("bucket").setKey("key"),
            filePath,
            [ar](GetObjectOutcome outcome) { ar->set(std::move(outcome)); });

    auto outcome = ar->wait();
    EXPECT_TRUE(outcome.has_value());

    std::ifstream f(filePath, std::ios::binary);
    std::string downloaded((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    EXPECT_EQ(content, downloaded);
    std::remove(filePath.c_str());
}

} // namespace alibabacloud::oss2
