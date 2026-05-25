#include <gtest/gtest.h>
#include "MockServer.h"
#include "alibabacloud/oss2/Error.h"

#include <future>
#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class TimeoutTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/slow-read", [](const httplib::Request&, httplib::Response& res) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-TIMEOUT");
            res.set_content("late response", "application/octet-stream");
        });
    }
};

TEST_F(TimeoutTest, ConnectTimeout) {
    auto config = makeConfig();
    config.connectTimeout = 1000;
    // 192.0.2.1 is TEST-NET-1 (RFC 5737), non-routable
    config.endpoint = "http://192.0.2.1:12345";
    auto client = makeClient(config);

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("any-key");

    auto outcome = client.getObject(request);
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::ConnectionFailed) ||
                ec == make_error_code(TransportErrorCode::Timeout));
}

TEST_F(TimeoutTest, ReadTimeout) {
    auto config = makeConfig();
    config.readWriteTimeout = 1000;
    auto client = makeClient(config);

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("slow-read");

    auto outcome = client.getObject(request);
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Timeout) ||
                ec == make_error_code(TransportErrorCode::SendRecvError));
}

TEST_F(TimeoutTest, AsyncConnectTimeout) {
    auto config = makeConfig();
    config.connectTimeout = 1000;
    config.endpoint = "http://192.0.2.1:12345";
    auto client = makeAsyncClient(config);

    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("any-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::ConnectionFailed) ||
                ec == make_error_code(TransportErrorCode::Timeout));
}

TEST_F(TimeoutTest, AsyncReadTimeout) {
    auto config = makeConfig();
    config.readWriteTimeout = 1000;
    auto client = makeAsyncClient(config);

    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("slow-read");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    auto ec = outcome.error().getErrorCode();
    EXPECT_TRUE(ec == make_error_code(TransportErrorCode::Timeout) ||
                ec == make_error_code(TransportErrorCode::SendRecvError));
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
