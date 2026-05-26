#include <gtest/gtest.h>

#include "src/transport/curl/CurlHttpClient.h"
#include "src/transport/curl/CurlMultiTransport.h"

using namespace alibabacloud::oss2;
using namespace alibabacloud::oss2::transport::curl;

TEST(CurlTransportTest, CurlHttpClientWithHttpTransportOptions) {
    HttpTransportOptions opts;
    opts.connectTimeout = 5000;
    opts.readWriteTimeout = 10000;
    CurlHttpClient client(opts);
    EXPECT_TRUE(client.getName().find("curl/") != std::string::npos);
}

TEST(CurlTransportTest, CurlMultiTransportWithHttpTransportOptions) {
    HttpTransportOptions opts;
    opts.connectTimeout = 5000;
    opts.readWriteTimeout = 10000;
    CurlMultiTransport transport(opts);
    EXPECT_TRUE(transport.getName().find("curl-multi/") != std::string::npos);
}
