#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/retry/BackoffDelayer.h"
#include "alibabacloud/oss2/retry/StandardRetryer.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <thread>

namespace alibabacloud::oss2 {

class MockTransportEx : public HttpTransport {
  public:
    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        auto req = std::make_unique<RequestMessage>(*request);
        if (req->body != nullptr) {
            auto src = req->body->spanSource();
            src->readToEnd();
        }
        requests.emplace_back(std::move(req));

        if (delay > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(delay);
        }

        if (options.cancellationToken.has_value() && options.cancellationToken->isCanceled()) {
            return TransportError{make_error_code(TransportErrorCode::Canceled),
                                  "RequestCanceled", "Request canceled by CancellationToken"};
        }

        if (isAborted && isAborted()) {
            return TransportError{make_error_code(TransportErrorCode::Canceled),
                                  "RequestCanceled", "Request canceled by CancellationToken"};
        }

        if (!responses.empty()) {
            auto res = std::move(responses.front());
            responses.erase(responses.begin());
            return res;
        }
        return TransportError{std::make_error_code(std::errc::result_out_of_range)};
    }
    std::string getName() const override { return "MockTransportEx"; }

    std::vector<ResponseResult> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    std::chrono::milliseconds delay{0};
    std::function<bool()> isAborted;
};

TEST(OSSClientMiscTest, TransportCanceled_NoRetry) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    for (int i = 0; i < 3; i++) {
        mock->responses.emplace_back(TransportError{
                make_error_code(TransportErrorCode::Canceled),
                "RequestCanceled", "Request canceled by CancellationToken"});
    }

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(1ULL, mock->requests.size());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSClientMiscTest, TransportCanceled_OperationErrorFields) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(TransportError{
            make_error_code(TransportErrorCode::Canceled),
            "RequestCanceled", "Request canceled by CancellationToken"});

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));

    EXPECT_FALSE(outcome.has_value());
    auto& error = outcome.error();
    EXPECT_EQ("PutObject", error.getOpName());
    EXPECT_EQ("PUT", error.getMethod());
    EXPECT_EQ(0, error.getStatusCode());
    EXPECT_EQ("RequestCanceled", error.getCode());
    EXPECT_EQ("Request canceled by CancellationToken", error.getMessage());
    EXPECT_EQ(error.getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSClientMiscTest, CancelToken_AlreadyCanceled) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();
    cts->cancel();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")),
            &opts);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSClientMiscTest, CancelToken_CancelDuringRequest) {
    auto mock = std::make_shared<MockTransportEx>();
    mock->delay = std::chrono::milliseconds(200);

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::thread canceller([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cts->cancel();
    });

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")),
            &opts);
    canceller.join();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSClientMiscTest, CancelToken_CancelAfterTimeout) {
    auto mock = std::make_shared<MockTransportEx>();
    mock->delay = std::chrono::milliseconds(200);

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    auto cts = CancellationTokenSource::create();
    cts->cancelAfter(std::chrono::milliseconds(50));

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")),
            &opts);

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestCanceled", outcome.error().getCode());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
}

TEST(OSSClientMiscTest, DisableRequestProcessing_BeforeRequest) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    client.disableRequest();

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ("RequestDisabled", outcome.error().getCode());
    EXPECT_EQ("Request disabled by client", outcome.error().getMessage());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);
    EXPECT_TRUE(mock->requests.empty());
}

TEST(OSSClientMiscTest, DisableRequestProcessing_EnableThenRequest) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    client.disableRequest();
    client.enableRequest();

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));

    EXPECT_TRUE(outcome.has_value());
}

TEST(OSSClientMiscTest, DisableRequestProcessing_DuringRequest) {
    auto mock = std::make_shared<MockTransportEx>();
    mock->delay = std::chrono::milliseconds(200);

    auto disableFlag = std::make_shared<std::atomic<bool>>(false);
    mock->isAborted = [disableFlag]() { return disableFlag->load(); };

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {}, nullptr}));

    std::thread disabler([&client, &disableFlag]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        disableFlag->store(true);
        client.disableRequest();
    });

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));
    disabler.join();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client.enableRequest();
}

TEST(OSSClientMiscTest, DisableRequestProcessing_DuringRetryWait) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;
    config.retryer = std::make_shared<StandardRetryer>(
            3, std::make_unique<FixedDelayBackoff>(std::chrono::milliseconds(500)));

    auto client = OSSClient(config);

    for (int i = 0; i < 3; i++) {
        mock->responses.emplace_back(TransportError{
                make_error_code(TransportErrorCode::ConnectionFailed),
                "ConnectError", "Connection failed"});
    }

    std::thread disabler([&client]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client.disableRequest();
    });

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));
    disabler.join();

    EXPECT_FALSE(outcome.has_value());
    EXPECT_EQ(outcome.error().getErrorCode(), ErrorCondition::Canceled);

    client.enableRequest();
}

TEST(OSSClientMiscTest, DisableRequestProcessing_RepeatedDisableEnable) {
    auto mock = std::make_shared<MockTransportEx>();

    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = mock;

    auto client = OSSClient(config);

    client.disableRequest();
    client.enableRequest();
    client.disableRequest();
    client.enableRequest();

    mock->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK", {{"x-oss-request-id", "id-1234"}}, nullptr}));

    auto outcome = client.putObject(
            models::PutObjectRequest()
                    .setBucket("bucket")
                    .setKey("key")
                    .setBody(RequestBody::FromString("data")));

    EXPECT_TRUE(outcome.has_value());
}

} // namespace alibabacloud::oss2
