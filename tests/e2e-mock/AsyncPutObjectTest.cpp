#include <gtest/gtest.h>
#include "MockServer.h"

#include <future>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class AsyncPutObjectTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Put("/bucket/test-key", [this](const httplib::Request& req, httplib::Response& res) {
            capturedBody_ = req.body;
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-APUT-001");
            res.set_header("ETag", "\"ASYNC_ETAG\"");
        });

        server().Put("/bucket/error-key", [](const httplib::Request&, httplib::Response& res) {
            xmlErrorResponse(res, 500, "InternalError", "Async internal error", "REQ-APUT-ERR");
        });
    }

    std::string capturedBody_;
};

TEST_F(AsyncPutObjectTest, AsyncPutObject_Success) {
    auto client = makeAsyncClient();
    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setBody(RequestBody::FromString("async hello"));

    client.putObjectAsync(request, [&promise](PutObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("REQ-APUT-001", outcome.value().getRequestId());
}

TEST_F(AsyncPutObjectTest, AsyncPutObject_BodyReceived) {
    auto client = makeAsyncClient();
    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    std::string content = "async body content";
    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setBody(RequestBody::FromString(content));

    client.putObjectAsync(request, [&promise](PutObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content, capturedBody_);
}

TEST_F(AsyncPutObjectTest, AsyncPutObject_ServerError) {
    auto client = makeAsyncClient();
    std::promise<PutObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("error-key")
        .setBody(RequestBody::FromString("data"));

    client.putObjectAsync(request, [&promise](PutObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("InternalError", outcome.error().getCode());
    EXPECT_EQ(500, outcome.error().getStatusCode());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
