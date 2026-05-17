#include <gtest/gtest.h>

#include "alibabacloud/oss2/Config.h"
#include "src/transport/HttpTransportFactory.h"

using namespace alibabacloud::oss2;

TEST(HttpTransportFactoryTest, CreateReturnsNonNull) {
    HttpTransportOptions opts;
    auto transport = transport::HttpTransportFactory::create(opts);
    ASSERT_NE(transport, nullptr);
}

TEST(HttpTransportFactoryTest, CreateReturnsExpectedTransport) {
    HttpTransportOptions opts;
    auto transport = transport::HttpTransportFactory::create(opts);
#if defined(ALIBABACLOUD_OSS_HAS_CURL)
    EXPECT_TRUE(transport->getName().find("curl") != std::string::npos);
#elif defined(ALIBABACLOUD_OSS_HAS_WINHTTP)
    EXPECT_EQ(transport->getName(), "winhttp");
#else
    EXPECT_EQ(transport->getName(), "NopHttpTransport");
#endif
}

TEST(HttpTransportFactoryTest, CreateWithOptions) {
    HttpTransportOptions opts;
    opts.connectTimeout = 3000;
    opts.readWriteTimeout = 8000;
    opts.insecureSkipVerify = true;
    auto transport = transport::HttpTransportFactory::create(opts);
    ASSERT_NE(transport, nullptr);
}

TEST(AsyncHttpTransportFactoryTest, CreateReturnsNonNull) {
    HttpTransportOptions opts;
    auto transport = transport::AsyncHttpTransportFactory::create(opts);
    ASSERT_NE(transport, nullptr);
}

TEST(AsyncHttpTransportFactoryTest, CreateReturnsExpectedTransport) {
    HttpTransportOptions opts;
    auto transport = transport::AsyncHttpTransportFactory::create(opts);
#if defined(ALIBABACLOUD_OSS_HAS_CURL)
    EXPECT_TRUE(transport->getName().find("curl-multi") != std::string::npos);
#elif defined(ALIBABACLOUD_OSS_HAS_WINHTTP)
    EXPECT_EQ(transport->getName(), "winhttp-async");
#else
    EXPECT_EQ(transport->getName(), "NopAsyncHttpTransport");
#endif
}
