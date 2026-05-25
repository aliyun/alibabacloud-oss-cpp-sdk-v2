#include <gtest/gtest.h>
#include "MockServer.h"

#include <future>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class AsyncMiscTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        server().Get("/bucket/head-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-AHEAD-001");
            res.set_header("Content-Length", "512");
            res.set_header("Content-Type", "application/json");
            res.set_content("dummy", "application/json");
        });

        server().Delete("/bucket/delete-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 204;
            res.set_header("x-oss-request-id", "REQ-ADEL-001");
        });

        server().Post("/bucket/", [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("delete")) {
                res.status = 200;
                res.set_header("x-oss-request-id", "REQ-ADELMULTI-001");
                res.set_content(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<DeleteResult>"
                    "<Deleted><Key>akey1</Key></Deleted>"
                    "</DeleteResult>",
                    "application/xml");
            }
        });
    }
};

TEST_F(AsyncMiscTest, AsyncHeadObject_Success) {
    auto client = makeAsyncClient();
    std::promise<HeadObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::HeadObjectRequest()
        .setBucket("bucket")
        .setKey("head-key");

    client.headObjectAsync(request, [&promise](HeadObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("REQ-AHEAD-001", outcome.value().getRequestId());
    EXPECT_EQ("application/json", outcome.value().getContentType());
}

TEST_F(AsyncMiscTest, AsyncDeleteObject_Success) {
    auto client = makeAsyncClient();
    std::promise<DeleteObjectOutcome> promise;
    auto future = promise.get_future();

    auto request = models::DeleteObjectRequest()
        .setBucket("bucket")
        .setKey("delete-key");

    client.deleteObjectAsync(request, [&promise](DeleteObjectOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
}

TEST_F(AsyncMiscTest, AsyncDeleteMultipleObjects_Success) {
    auto client = makeAsyncClient();
    std::promise<DeleteMultipleObjectsOutcome> promise;
    auto future = promise.get_future();

    auto request = models::DeleteMultipleObjectsRequest()
        .setBucket("bucket")
        .setDelete(models::Delete().setObjects({
            models::ObjectIdentifier().setKey("akey1")
        }));

    client.deleteMultipleObjectsAsync(request, [&promise](DeleteMultipleObjectsOutcome outcome) {
        promise.set_value(std::move(outcome));
    });

    auto outcome = future.get();
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(1ULL, outcome.value().getDeletedObjects().size());
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
