#include <gtest/gtest.h>

#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/Error.h"

using namespace alibabacloud::oss2;

TEST(NopTransportTest, NopHttpTransportSend) {
    NopHttpTransport transport;
    EXPECT_EQ("NopHttpTransport", transport.getName());

    auto request = std::make_unique<RequestMessage>();
    RequestOptions opts;
    auto result = transport.send(request, opts);
    ASSERT_TRUE(std::holds_alternative<TransportError>(result));
    auto& err = std::get<TransportError>(result);
    EXPECT_EQ(make_error_code(TransportErrorCode::NotSupported), err.error);
}

TEST(NopTransportTest, NopAsyncHttpTransportSendAsync) {
    NopAsyncHttpTransport transport;
    EXPECT_EQ("NopAsyncHttpTransport", transport.getName());

    auto request = std::make_unique<RequestMessage>();
    RequestOptions opts;
    bool callbackCalled = false;
    transport.sendAsync(std::move(request), opts,
        [&callbackCalled](ResponseResult result, std::unique_ptr<RequestMessage>) {
            callbackCalled = true;
            ASSERT_TRUE(std::holds_alternative<TransportError>(result));
            auto& err = std::get<TransportError>(result);
            EXPECT_EQ(make_error_code(TransportErrorCode::NotSupported), err.error);
        });
    EXPECT_TRUE(callbackCalled);
}
