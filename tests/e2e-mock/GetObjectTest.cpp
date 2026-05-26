#include <gtest/gtest.h>
#include "MockServer.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class GetObjectTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/test-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-GET-001");
            res.set_header("ETag", "\"ABCDEF1234567890\"");
            res.set_content("hello from server", "application/octet-stream");
        });

        server().Get("/bucket/large-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-GET-LARGE");
            std::string largeBody(1024 * 1024, 'X');
            res.set_content(largeBody, "application/octet-stream");
        });

        server().Get("/bucket/empty-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-GET-EMPTY");
        });

        server().Get("/bucket/missing-key", [](const httplib::Request&, httplib::Response& res) {
            xmlErrorResponse(res, 404, "NoSuchKey", "The specified key does not exist.", "REQ-GET-404");
        });
    }
};

TEST_F(GetObjectTest, GetObject_Success) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("test-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("REQ-GET-001", outcome.value().getRequestId());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ("hello from server", content);
}

TEST_F(GetObjectTest, GetObject_WithSinkFactory) {
    auto client = makeClient();
    auto sinkStream = std::make_shared<std::stringstream>();

    SinkFactory factory;
    factory.isOneShot = true;
    factory.supplier = [sinkStream](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(sinkStream);
    };

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("test-key")
        .setSinkFactory(factory);

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ("hello from server", sinkStream->str());
}

TEST_F(GetObjectTest, GetObject_ErrorResponse) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("missing-key");

    auto outcome = client.getObject(request);
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("NoSuchKey", outcome.error().getCode());
    EXPECT_EQ(404, outcome.error().getStatusCode());
    EXPECT_EQ("REQ-GET-404", outcome.error().getRequestId());
}

TEST_F(GetObjectTest, GetObject_LargeBody) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("large-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ(1024 * 1024, static_cast<int>(content.size()));
    EXPECT_TRUE(content.find_first_not_of('X') == std::string::npos);
}

TEST_F(GetObjectTest, GetObject_EmptyBody) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("empty-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
