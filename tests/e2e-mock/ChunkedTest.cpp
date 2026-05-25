#include <gtest/gtest.h>
#include "MockServer.h"

#include <future>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class ChunkedTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        // Chunked response: use content provider without length
        server().Get("/bucket/chunked-key", [](const httplib::Request&, httplib::Response& res) {
            std::string data = "chunk1-data chunk2-data chunk3-data";
            size_t sent = 0;
            res.set_content_provider(
                "application/octet-stream",
                [data, sent](size_t offset, httplib::DataSink& sink) mutable -> bool {
                    const size_t chunkSize = 11;
                    if (offset >= data.size()) {
                        sink.done();
                        return true;
                    }
                    size_t remaining = data.size() - offset;
                    size_t toSend = (std::min)(chunkSize, remaining);
                    sink.write(data.c_str() + offset, toSend);
                    return true;
                });
            res.set_header("x-oss-request-id", "REQ-CHUNKED-001");
        });

        // Large chunked response (~1MB)
        server().Get("/bucket/chunked-large", [](const httplib::Request&, httplib::Response& res) {
            const size_t totalSize = 1024 * 1024;
            const size_t chunkSize = 4096;
            res.set_content_provider(
                "application/octet-stream",
                [totalSize, chunkSize](size_t offset, httplib::DataSink& sink) -> bool {
                    if (offset >= totalSize) {
                        sink.done();
                        return true;
                    }
                    size_t remaining = totalSize - offset;
                    size_t toSend = (std::min)(chunkSize, remaining);
                    std::string chunk(toSend, 'Y');
                    sink.write(chunk.c_str(), toSend);
                    return true;
                });
            res.set_header("x-oss-request-id", "REQ-CHUNKED-LARGE");
        });
    }
};

TEST_F(ChunkedTest, ChunkedResponse) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("chunked-key");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ("chunk1-data chunk2-data chunk3-data", content);
}

TEST_F(ChunkedTest, ChunkedLargeResponse) {
    auto client = makeClient();
    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("chunked-large");

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ(1024 * 1024, static_cast<int>(content.size()));
    EXPECT_TRUE(content.find_first_not_of('Y') == std::string::npos);
}

TEST_F(ChunkedTest, AsyncChunkedResponse) {
    auto client = makeAsyncClient();
    std::promise<GetObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::GetObjectRequest()
        .setBucket("bucket")
        .setKey("chunked-key");

    client.getObjectAsync(request, [&promise](GetObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());

    auto& body = outcome.value().getBody();
    ASSERT_NE(nullptr, body);
    std::string content((std::istreambuf_iterator<char>(*body)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ("chunk1-data chunk2-data chunk3-data", content);
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
