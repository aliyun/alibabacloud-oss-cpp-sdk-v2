#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <thread>

namespace alibabacloud::oss2 {

class MockAsyncTransportEx : public AsyncHttpTransport {
  public:
    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options, RequestCallback callback) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto req = std::make_unique<RequestMessage>(*request);
            if (req->body != nullptr) {
                auto src = req->body->spanSource();
                src->readToEnd();
            }
            requests.emplace_back(std::move(req));
        }

        if (delay > std::chrono::milliseconds(0)) {
            auto token = options.cancellationToken;
            ResponseResult result = popResponse();
            std::thread([this, token, result = std::move(result),
                         request = std::move(request), callback = std::move(callback)]() mutable {
                std::this_thread::sleep_for(delay);
                if (token.has_value() && token->isCanceled()) {
                    callback(TransportError{make_error_code(TransportErrorCode::Canceled),
                                            "RequestCanceled", "Request canceled by CancellationToken"},
                             std::move(request));
                } else {
                    callback(std::move(result), std::move(request));
                }
            }).detach();
            return;
        }

        if (options.cancellationToken.has_value() && options.cancellationToken->isCanceled()) {
            callback(TransportError{make_error_code(TransportErrorCode::Canceled),
                                    "RequestCanceled", "Request canceled by CancellationToken"},
                     std::move(request));
            return;
        }

        ResponseResult result = popResponse();
        callback(std::move(result), std::move(request));
    }
    std::string getName() const override { return "MockAsyncTransportEx"; }

    std::vector<ResponseResult> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    std::chrono::milliseconds delay{0};
    std::mutex mutex_;

  private:
    ResponseResult popResponse() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return TransportError{std::make_error_code(std::errc::result_out_of_range)};
    }
};

TEST(OSSAsyncClientMiscTest, TransportCanceled_NoRetry) {
    auto mock = std::make_shared<MockAsyncTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    for (int i = 0; i < 3; i++) {
        mock->responses.emplace_back(TransportError{
                make_error_code(TransportErrorCode::Canceled),
                "RequestCanceled", "Request canceled by CancellationToken"});
    }

    auto future = client.asyncCall(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::fromString("data")));
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(1ULL, mock->requests.size());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSAsyncClientMiscTest, TransportCanceled_OperationErrorFields) {
    auto mock = std::make_shared<MockAsyncTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    mock->responses.emplace_back(TransportError{
            make_error_code(TransportErrorCode::Canceled),
            "RequestCanceled", "Request canceled by CancellationToken"});

    auto future = client.asyncCall(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::fromString("data")));
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("PutObject", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("RequestCanceled", error.getCode());
    EXPECT_EQ("Request canceled by CancellationToken", error.getMessage());
    EXPECT_EQ(error.getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSAsyncClientMiscTest, CancelToken_AlreadyCanceled) {
    auto mock = std::make_shared<MockAsyncTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();
    cts->cancel();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client.asyncCall(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::fromString("data")),
            &opts);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSAsyncClientMiscTest, CancelToken_CancelDuringRequest) {
    auto mock = std::make_shared<MockAsyncTransportEx>();
    mock->delay = std::chrono::milliseconds(200);

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client.asyncCall(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::fromString("data")),
            &opts);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    cts->cancel();

    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSAsyncClientMiscTest, CancelToken_CancelAfterTimeout) {
    auto mock = std::make_shared<MockAsyncTransportEx>();
    mock->delay = std::chrono::milliseconds(200);

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::milliseconds(50));

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto future = client.asyncCall(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::fromString("data")),
            &opts);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

namespace {

class FailingWriter : public ByteWriter {
  public:
    explicit FailingWriter(std::size_t failAfter) : failAfter_(failAfter) {}
    std::string data;

  private:
    std::size_t onWrite(const std::uint8_t* d, std::size_t n) override {
        if (written_ >= failAfter_) return 0;
        std::size_t canWrite = (std::min)(n, failAfter_ - written_);
        data.append(reinterpret_cast<const char*>(d), canWrite);
        written_ += canWrite;
        return canWrite;
    }
    int iostate() const override {
        return written_ >= failAfter_ ? std::ios_base::badbit : 0;
    }
    std::size_t written_{0};
    std::size_t failAfter_;
};

class WritingMockAsyncTransport : public AsyncHttpTransport {
  public:
    struct Response {
        int statusCode{200};
        HeaderCollection headers;
        std::string body;
    };
    std::vector<Response> responses;

    void sendAsync(std::unique_ptr<RequestMessage> request,
                   const RequestOptions& options,
                   RequestCallback callback) override {
        if (request->body != nullptr) {
            auto src = request->body->spanSource();
            src->readToEnd();
        }

        Response r;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            r = std::move(responses.front());
            responses.erase(responses.begin());
        }

        auto response = std::make_unique<ResponseMessage>();
        response->statusCode = r.statusCode;
        response->headers = r.headers;

        bool isError = (r.statusCode / 100 != 2) || (r.statusCode == 203);

        if (!isError && options.sinkFactory.has_value()) {
            auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(r.body.size()), response->headers);
            if (sink) {
                auto* data = reinterpret_cast<const std::uint8_t*>(r.body.data());
                sink->write(data, r.body.size());
                if (sink->bad()) {
                    callback(TransportError{make_error_code(TransportErrorCode::SendRecvError),
                                            "WriteStreamError", "Failed to write response body"},
                             std::move(request));
                    return;
                }
            }
        } else {
            response->body = std::make_shared<std::stringstream>(r.body);
        }

        callback(std::move(response), std::move(request));
    }

    std::string getName() const override { return "WritingMockAsyncTransport"; }

  private:
    std::mutex mutex_;
};

} // namespace

TEST(OSSAsyncClientMiscTest, GetObject_ObservableWriter_Success) {
    auto mock = std::make_shared<WritingMockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    std::string responseBody = "hello observable writer";
    mock->responses.push_back({200, {{"x-oss-request-id", "id-1234"},
                                     {"Content-Length", std::to_string(responseBody.size())}},
                               responseBody});

    auto output = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(output);
    auto crc = std::make_shared<CRC64WriteObserver>();

    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;
    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred,
                            std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(responseBody.size()), total);
    };
    auto progressObs = std::make_shared<ProgressWriteObserver>(progress,
            static_cast<std::int64_t>(responseBody.size()));

    auto sink = std::make_shared<ObservableWriter>(writer, progressObs, crc);

    SinkFactory factory;
    factory.supplier = [sink](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> { return sink; };
    factory.isOneShot = false;

    auto future = client.asyncCall(
            models::GetObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setSinkFactory(factory));
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ(responseBody, output->str());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(responseBody.size(), lastTransferred);
    EXPECT_EQ(responseBody.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, responseBody.data(), responseBody.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST(OSSAsyncClientMiscTest, GetObject_ObservableWriter_RetryWithReset) {
    auto mock = std::make_shared<WritingMockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;
    config.retryer = std::make_shared<StandardRetryer>(
            3, std::make_unique<FixedDelayBackoff>(std::chrono::milliseconds(0)));

    auto client = OSSAsyncClient(config);

    std::string firstBody = "this body is too long for failing writer";
    std::string secondBody = "retry ok";

    mock->responses.push_back({200, {{"x-oss-request-id", "id-1"},
                                     {"Content-Length", std::to_string(firstBody.size())}},
                               firstBody});
    mock->responses.push_back({200, {{"x-oss-request-id", "id-2"},
                                     {"Content-Length", std::to_string(secondBody.size())}},
                               secondBody});

    std::vector<std::size_t> progressIncrements;
    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t, std::int64_t, std::uintptr_t) {
        progressIncrements.push_back(increment);
    };
    auto progressObs = std::make_shared<ProgressWriteObserver>(progress, -1);
    auto crc = std::make_shared<CRC64WriteObserver>();

    int supplierCallCount = 0;
    std::shared_ptr<ObservableWriter> currentSink;

    SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [&](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        if (supplierCallCount == 1) {
            auto failWriter = std::make_shared<FailingWriter>(5);
            currentSink = std::make_shared<ObservableWriter>(failWriter, progressObs, crc);
            return currentSink;
        }
        progressObs->reset();
        crc->reset();
        auto goodOutput = std::make_shared<std::ostringstream>();
        auto goodWriter = std::make_shared<OStreamWriter>(goodOutput);
        currentSink = std::make_shared<ObservableWriter>(goodWriter, progressObs, crc);
        return currentSink;
    };

    auto future = client.asyncCall(
            models::GetObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setSinkFactory(factory));
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(2, supplierCallCount);

    uint64_t expectedCrc = utils::CalcCRC64(0, secondBody.data(), secondBody.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST(OSSAsyncClientMiscTest, GetObject_ObservableWriter_LargeBody) {
    auto mock = std::make_shared<WritingMockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    std::string responseBody(4096, 'X');
    mock->responses.push_back({200, {{"x-oss-request-id", "id-1234"},
                                     {"Content-Length", std::to_string(responseBody.size())}},
                               responseBody});

    auto output = std::make_shared<std::ostringstream>();
    auto writer = std::make_shared<OStreamWriter>(output);

    std::size_t totalIncrement = 0;
    std::size_t lastTransferred = 0;
    int callCount = 0;
    ProgressCallback progress;
    progress.callback = [&](std::size_t increment, std::size_t transferred,
                            std::int64_t total, std::uintptr_t) {
        totalIncrement += increment;
        lastTransferred = transferred;
        callCount++;
        EXPECT_EQ(static_cast<std::int64_t>(responseBody.size()), total);
    };
    auto progressObs = std::make_shared<ProgressWriteObserver>(progress,
            static_cast<std::int64_t>(responseBody.size()));
    auto crc = std::make_shared<CRC64WriteObserver>();

    auto sink = std::make_shared<ObservableWriter>(writer, progressObs, crc);

    SinkFactory factory;
    factory.supplier = [sink](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> { return sink; };

    auto future = client.asyncCall(
            models::GetObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setSinkFactory(factory));
    auto outcome = future.get();

    EXPECT_TRUE(outcome.has_value());
    EXPECT_EQ(responseBody, output->str());
    EXPECT_GT(callCount, 0);
    EXPECT_EQ(responseBody.size(), lastTransferred);
    EXPECT_EQ(responseBody.size(), totalIncrement);

    uint64_t expectedCrc = utils::CalcCRC64(0, responseBody.data(), responseBody.size());
    EXPECT_EQ(expectedCrc, crc->crc());
}

TEST(OSSAsyncClientMiscTest, GetObject_ObservableWriter_ErrorResponse_SinkNotInvoked) {
    auto mock = std::make_shared<WritingMockAsyncTransport>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.asyncHttpTransport = mock;

    auto client = OSSAsyncClient(config);

    std::string errorBody = R"(<Error>
    <Code>NoSuchKey</Code>
    <Message>The specified key does not exist</Message>
    <RequestId>id-1234</RequestId>
    <HostId>oss-cn-hangzhou.aliyuncs.com</HostId>
</Error>)";

    mock->responses.push_back({404, {{"x-oss-request-id", "id-1234"}}, errorBody});

    int supplierCallCount = 0;
    auto crc = std::make_shared<CRC64WriteObserver>();

    SinkFactory factory;
    factory.supplier = [&](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        supplierCallCount++;
        auto output = std::make_shared<std::ostringstream>();
        auto writer = std::make_shared<OStreamWriter>(output);
        return std::make_shared<ObservableWriter>(writer, crc);
    };

    auto future = client.asyncCall(
            models::GetObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setSinkFactory(factory));
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    EXPECT_EQ(0, supplierCallCount);
    EXPECT_EQ(0ULL, crc->crc());
}

} // namespace alibabacloud::oss2
