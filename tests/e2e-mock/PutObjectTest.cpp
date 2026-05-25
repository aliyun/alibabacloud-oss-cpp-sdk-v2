#include <gtest/gtest.h>
#include "MockServer.h"

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class PutObjectTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Put("/bucket/test-key", [this](const httplib::Request& req, httplib::Response& res) {
            capturedBody_ = req.body;
            capturedHeaders_ = req.headers;
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-PUT-001");
            res.set_header("ETag", "\"D41D8CD98F00B204E9800998ECF8427E\"");
        });

        server().Put("/bucket/error-key", [](const httplib::Request&, httplib::Response& res) {
            xmlErrorResponse(res, 500, "InternalError", "Internal server error", "REQ-PUT-ERR");
        });
    }

    std::string capturedBody_;
    httplib::Headers capturedHeaders_;
};

TEST_F(PutObjectTest, PutObject_Success) {
    auto client = makeClient();
    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setBody(RequestBody::FromString("hello world"));

    auto outcome = client.putObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("REQ-PUT-001", outcome.value().getRequestId());
}

TEST_F(PutObjectTest, PutObject_BodyReceived) {
    auto client = makeClient();
    std::string content = "test body content 12345";
    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setBody(RequestBody::FromString(content));

    auto outcome = client.putObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(content, capturedBody_);
}

TEST_F(PutObjectTest, PutObject_HeadersForwarded) {
    auto client = makeClient();
    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setBody(RequestBody::FromString("data"))
        .setMetadata({{"Content-Type", "text/plain"}, {"custom", "value1"}});

    auto outcome = client.putObject(request);
    ASSERT_TRUE(outcome.has_value());
    auto it = capturedHeaders_.find("x-oss-meta-custom");
    ASSERT_NE(it, capturedHeaders_.end());
    EXPECT_EQ("value1", it->second);
}

TEST_F(PutObjectTest, PutObject_ServerError) {
    auto client = makeClient();
    auto request = models::PutObjectRequest()
        .setBucket("bucket")
        .setKey("error-key")
        .setBody(RequestBody::FromString("data"));

    auto outcome = client.putObject(request);
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("InternalError", outcome.error().getCode());
    EXPECT_EQ(500, outcome.error().getStatusCode());
    EXPECT_EQ("REQ-PUT-ERR", outcome.error().getRequestId());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
