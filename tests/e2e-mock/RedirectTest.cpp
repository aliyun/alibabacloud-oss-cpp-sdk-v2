#include <gtest/gtest.h>
#include "MockServer.h"

#include <future>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class RedirectTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/redirect301-key", [this](const httplib::Request&, httplib::Response& res) {
            res.status = 301;
            res.set_header("Location", "http://127.0.0.1:" + std::to_string(port()) + "/bucket/final-key");
        });

        server().Get("/bucket/redirect302-key", [this](const httplib::Request&, httplib::Response& res) {
            res.status = 302;
            res.set_header("Location", "http://127.0.0.1:" + std::to_string(port()) + "/bucket/final-key");
        });

        server().Get("/bucket/final-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-REDIRECT-FINAL");
            res.set_content("redirected content", "application/octet-stream");
        });
    }
};

TEST_F(RedirectTest, Redirect301) {
    auto config = makeConfig();
    config.enabledRedirect = true;
    auto client = makeClient(config);

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("redirect301-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ("redirected content", content);
}

TEST_F(RedirectTest, Redirect302) {
    auto config = makeConfig();
    config.enabledRedirect = true;
    auto client = makeClient(config);

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("redirect302-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

// Async transport does not currently support enabledRedirect,
// so async redirect returns a non-200 response
TEST_F(RedirectTest, AsyncRedirect301_NotFollowed) {
    auto config = makeConfig();
    config.enabledRedirect = true;
    auto client = makeAsyncClient(config);

    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("redirect301-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_FALSE(outcome.has_value());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
