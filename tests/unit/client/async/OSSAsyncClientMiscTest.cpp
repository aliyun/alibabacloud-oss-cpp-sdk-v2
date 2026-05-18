#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
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
                    .setBody(RequestBody::FromString("data")));
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
                    .setBody(RequestBody::FromString("data")));
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
                    .setBody(RequestBody::FromString("data")),
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
                    .setBody(RequestBody::FromString("data")),
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
                    .setBody(RequestBody::FromString("data")),
            &opts);
    auto outcome = future.get();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

} // namespace alibabacloud::oss2
