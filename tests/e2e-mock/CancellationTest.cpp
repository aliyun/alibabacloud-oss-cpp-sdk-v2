#include <gtest/gtest.h>
#include "MockServer.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <future>
#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class CancellationTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Put("/bucket/slow-key", [](const httplib::Request&, httplib::Response& res) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-CANCEL-PUT");
        });

        server().Get("/bucket/slow-key", [](const httplib::Request&, httplib::Response& res) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-CANCEL-GET");
            res.set_content("delayed body", "application/octet-stream");
        });
    }
};

TEST_F(CancellationTest, PutObject_Canceled) {
    auto client = makeClient();
    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::thread cancelThread([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cts->cancel();
    });

    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("slow-key")
        .setBody(RequestBody::fromString("data"));

    auto outcome = client.putObject(request, &opts);
    cancelThread.join();

    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Canceled) ||
                ec == make_error_code(ClientErrorCode::OperationCanceled));
}

TEST_F(CancellationTest, GetObject_Canceled) {
    auto client = makeClient();
    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::thread cancelThread([&cts]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cts->cancel();
    });

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("slow-key");

    auto outcome = client.getObject(request, &opts);
    cancelThread.join();

    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Canceled) ||
                ec == make_error_code(ClientErrorCode::OperationCanceled));
}

TEST_F(CancellationTest, AsyncPutObject_Canceled) {
    auto client = makeAsyncClient();
    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("slow-key")
        .setBody(RequestBody::fromString("data"));

    client.putObjectAsync(request, [&promise](PutObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    }, &opts);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cts->cancel();

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Canceled) ||
                ec == make_error_code(ClientErrorCode::OperationCanceled));
}

TEST_F(CancellationTest, AsyncGetObject_Canceled) {
    auto client = makeAsyncClient();
    auto cts = CancellationTokenSource::create();

    OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("slow-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    }, &opts);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cts->cancel();

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Canceled) ||
                ec == make_error_code(ClientErrorCode::OperationCanceled));
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
