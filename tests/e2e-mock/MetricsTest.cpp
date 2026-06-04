#include <gtest/gtest.h>
#include "MockServer.h"
#include "src/transport/HttpTransportFactory.h"

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class MetricsTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/metrics-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-METRICS-001");
            res.set_content("metrics test body", "application/octet-stream");
        });

        server().Put("/bucket/metrics-put-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-METRICS-002");
        });
    }
};

TEST_F(MetricsTest, SyncTransport_MetricsEnabled) {
    HttpTransportOptions opts;
    opts.collectMetrics = true;
    auto transport = transport::HttpTransportFactory::create(opts);
    ASSERT_NE(transport, nullptr);

    auto request = std::make_unique<RequestMessage>();
    request->method = "GET";
    request->uri = "http://127.0.0.1:" + std::to_string(port()) + "/bucket/metrics-key";

    RequestOptions reqOpts;
    auto result = transport->send(request, reqOpts);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ResponseMessage>>(result));

    auto& response = std::get<std::unique_ptr<ResponseMessage>>(result);
    EXPECT_EQ(response->statusCode, 200);

    ASSERT_NE(response->metrics, nullptr);
    EXPECT_GT(response->metrics->total.count(), 0);
    EXPECT_GT(response->metrics->startTransfer.count(), 0);
    auto now = std::chrono::system_clock::now();
    EXPECT_LE(response->metrics->requestStart, now);
    EXPECT_GT(response->metrics->requestStart.time_since_epoch().count(), 0);
}

TEST_F(MetricsTest, SyncTransport_MetricsDisabled) {
    HttpTransportOptions opts;
    opts.collectMetrics = false;
    auto transport = transport::HttpTransportFactory::create(opts);

    auto request = std::make_unique<RequestMessage>();
    request->method = "GET";
    request->uri = "http://127.0.0.1:" + std::to_string(port()) + "/bucket/metrics-key";

    RequestOptions reqOpts;
    auto result = transport->send(request, reqOpts);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ResponseMessage>>(result));

    auto& response = std::get<std::unique_ptr<ResponseMessage>>(result);
    EXPECT_EQ(response->statusCode, 200);
    EXPECT_EQ(response->metrics, nullptr);
}

TEST_F(MetricsTest, AsyncTransport_MetricsEnabled) {
    HttpTransportOptions opts;
    opts.collectMetrics = true;
    auto transport = transport::AsyncHttpTransportFactory::create(opts);
    ASSERT_NE(transport, nullptr);

    auto request = std::make_unique<RequestMessage>();
    request->method = "GET";
    request->uri = "http://127.0.0.1:" + std::to_string(port()) + "/bucket/metrics-key";

    std::promise<ResponseResult> promise;
    auto future = promise.get_future();

    RequestOptions reqOpts;
    transport->sendAsync(std::move(request), reqOpts,
        [&promise](ResponseResult result, std::unique_ptr<RequestMessage>) {
            promise.set_value(std::move(result));
        });

    auto result = future.get();
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ResponseMessage>>(result));

    auto& response = std::get<std::unique_ptr<ResponseMessage>>(result);
    EXPECT_EQ(response->statusCode, 200);

    ASSERT_NE(response->metrics, nullptr);
    EXPECT_GT(response->metrics->total.count(), 0);
}

TEST_F(MetricsTest, SyncTransport_MetricsTimingOrder) {
    HttpTransportOptions opts;
    opts.collectMetrics = true;
    auto transport = transport::HttpTransportFactory::create(opts);

    auto request = std::make_unique<RequestMessage>();
    request->method = "GET";
    request->uri = "http://127.0.0.1:" + std::to_string(port()) + "/bucket/metrics-key";

    RequestOptions reqOpts;
    auto result = transport->send(request, reqOpts);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ResponseMessage>>(result));

    auto& m = std::get<std::unique_ptr<ResponseMessage>>(result)->metrics;
    ASSERT_NE(m, nullptr);
    EXPECT_LE(m->startTransfer, m->total);
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
