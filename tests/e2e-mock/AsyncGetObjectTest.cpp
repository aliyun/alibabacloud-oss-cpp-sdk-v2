#include <gtest/gtest.h>
#include "MockServer.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <future>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class AsyncGetObjectTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/test-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-AGET-001");
            res.set_content("async response body", "application/octet-stream");
        });

        server().Get("/bucket/missing-key", [](const httplib::Request&, httplib::Response& res) {
            xmlErrorResponse(res, 404, "NoSuchKey", "Key not found", "REQ-AGET-404");
        });
    }
};

TEST_F(AsyncGetObjectTest, AsyncGetObject_Success) {
    auto client = makeAsyncClient();
    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("test-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ("async response body", content);
}

TEST_F(AsyncGetObjectTest, AsyncGetObject_WithSinkFactory) {
    auto client = makeAsyncClient();
    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto sinkStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [sinkStream](std::int64_t) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(sinkStream);
    };

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setSinkFactory(factory);

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ("async response body", sinkStream->str());
}

TEST_F(AsyncGetObjectTest, AsyncGetObject_ErrorResponse) {
    auto client = makeAsyncClient();
    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("missing-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    EXPECT_EQ(404, outcome.error().getStatusCode());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
