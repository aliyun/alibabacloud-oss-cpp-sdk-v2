#pragma once

#include <gtest/gtest.h>
#include <httplib.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace e2e {

class MockServerFixture : public ::testing::Test {
  protected:
    void SetUp() override {
        server_ = std::make_unique<httplib::Server>();
        setupRoutes();
        port_ = server_->bind_to_any_port("127.0.0.1");
        listener_ = std::thread([this]() { server_->listen_after_bind(); });
        server_->wait_until_ready();
    }

    void TearDown() override {
        server_->stop();
        if (listener_.joinable()) listener_.join();
    }

    virtual void setupRoutes() {}

    OSSClient makeClient() {
        return OSSClient(makeConfig());
    }

    OSSClient makeClient(const ClientConfiguration& config) {
        return OSSClient(config);
    }

    OSSAsyncClient makeAsyncClient() {
        return OSSAsyncClient(makeConfig());
    }

    OSSAsyncClient makeAsyncClient(const ClientConfiguration& config) {
        return OSSAsyncClient(config);
    }

    ClientConfiguration makeConfig() {
        auto config = ClientConfiguration::loadDefault();
        config.region = "cn-hangzhou";
        config.endpoint = "http://127.0.0.1:" + std::to_string(port_);
        config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
        config.signatureVersion = "v1";
        return config;
    }

    httplib::Server& server() { return *server_; }
    int port() const { return port_; }

    static void xmlErrorResponse(httplib::Response& res, int status,
                                 const std::string& code,
                                 const std::string& message,
                                 const std::string& requestId) {
        res.status = status;
        res.set_header("x-oss-request-id", requestId);
        res.set_content(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<Error><Code>" + code + "</Code>"
            "<Message>" + message + "</Message>"
            "<RequestId>" + requestId + "</RequestId></Error>",
            "application/xml");
    }

  private:
    std::unique_ptr<httplib::Server> server_;
    std::thread listener_;
    int port_{0};
};

} // namespace e2e
} // namespace oss2
} // namespace alibabacloud
