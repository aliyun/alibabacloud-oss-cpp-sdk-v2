#include <gtest/gtest.h>
#include "MockServer.h"

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class MiscMethodTest : public MockServerFixture {
  protected:
    void setupRoutes() override {
        // httplib automatically handles HEAD from GET handlers (returns headers only)
        server().Get("/bucket/head-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_header("x-oss-request-id", "REQ-HEAD-001");
            res.set_header("Content-Length", "1024");
            res.set_header("ETag", "\"ETAG123\"");
            res.set_header("Content-Type", "text/plain");
            res.set_content("dummy", "text/plain");
        });

        server().Delete("/bucket/delete-key", [](const httplib::Request&, httplib::Response& res) {
            res.status = 204;
            res.set_header("x-oss-request-id", "REQ-DEL-001");
        });

        server().Delete("/bucket/forbidden-key", [](const httplib::Request&, httplib::Response& res) {
            xmlErrorResponse(res, 403, "AccessDenied", "Access denied", "REQ-DEL-403");
        });

        server().Post("/bucket/", [this](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("delete")) {
                capturedPostBody_ = req.body;
                res.status = 200;
                res.set_header("x-oss-request-id", "REQ-DELMULTI-001");
                res.set_content(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<DeleteResult>"
                    "<Deleted><Key>key1</Key></Deleted>"
                    "<Deleted><Key>key2</Key></Deleted>"
                    "</DeleteResult>",
                    "application/xml");
            }
        });
    }

    std::string capturedPostBody_;
};

TEST_F(MiscMethodTest, HeadObject_Success) {
    auto client = makeClient();
    auto request = models::HeadObjectRequest()
        .setBucket("bucket")
        .setKey("head-key");

    auto outcome = client.headObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(200, outcome.value().getStatusCode());
    EXPECT_EQ("REQ-HEAD-001", outcome.value().getRequestId());
    EXPECT_EQ("text/plain", outcome.value().getContentType());
}

TEST_F(MiscMethodTest, DeleteObject_Success) {
    auto client = makeClient();
    auto request = models::DeleteObjectRequest()
        .setBucket("bucket")
        .setKey("delete-key");

    auto outcome = client.deleteObject(request);
    ASSERT_TRUE(outcome.has_value());
}

TEST_F(MiscMethodTest, DeleteObject_ErrorResponse) {
    auto client = makeClient();
    auto request = models::DeleteObjectRequest()
        .setBucket("bucket")
        .setKey("forbidden-key");

    auto outcome = client.deleteObject(request);
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("AccessDenied", outcome.error().getCode());
    EXPECT_EQ(403, outcome.error().getStatusCode());
}

TEST_F(MiscMethodTest, DeleteMultipleObjects_Success) {
    auto client = makeClient();
    auto request = models::DeleteMultipleObjectsRequest()
        .setBucket("bucket")
        .setDelete(models::Delete().setObjects({
            models::ObjectIdentifier().setKey("key1"),
            models::ObjectIdentifier().setKey("key2")
        }));

    auto outcome = client.deleteMultipleObjects(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(2ULL, outcome.value().getDeletedObjects().size());
    EXPECT_FALSE(capturedPostBody_.empty());
}

TEST_F(MiscMethodTest, PostRequest_BodyForwarded) {
    auto client = makeClient();
    auto request = models::DeleteMultipleObjectsRequest()
        .setBucket("bucket")
        .setDelete(models::Delete().setObjects({
            models::ObjectIdentifier().setKey("obj1")
        }).setQuiet(true));

    auto outcome = client.deleteMultipleObjects(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_NE(std::string::npos, capturedPostBody_.find("obj1"));
}

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
