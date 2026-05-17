#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"

#include <system_error>

using namespace alibabacloud::oss2;

TEST(ErrorTest, ErrorCodeDefaultConstruction) {
    std::error_code ec;
    EXPECT_FALSE(ec);
    EXPECT_EQ(0, ec.value());
}

// --- ClientErrorCode ---

TEST(ErrorTest, ClientErrorCategory) {
    auto ec = make_error_code(ClientErrorCode::ArgumentRequired);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.client"), ec.category().name());
    EXPECT_EQ(static_cast<int>(ClientErrorCode::ArgumentRequired), ec.value());
}

TEST(ErrorTest, ClientErrorConditionInvalidArgument) {
    EXPECT_EQ(make_error_code(ClientErrorCode::ArgumentInvalid), ErrorCondition::InvalidArgument);
    EXPECT_EQ(make_error_code(ClientErrorCode::ArgumentRequired), ErrorCondition::InvalidArgument);
    EXPECT_EQ(make_error_code(ClientErrorCode::EndpointInvalid), ErrorCondition::InvalidArgument);
    EXPECT_EQ(make_error_code(ClientErrorCode::BucketNameInvalid), ErrorCondition::InvalidArgument);
    EXPECT_EQ(make_error_code(ClientErrorCode::ObjectNameInvalid), ErrorCondition::InvalidArgument);
    EXPECT_EQ(make_error_code(ClientErrorCode::RequestMethodEmpty), ErrorCondition::InvalidArgument);
}

TEST(ErrorTest, ClientErrorConditionRetryable) {
    EXPECT_EQ(make_error_code(ClientErrorCode::CrcMismatch), ErrorCondition::Retryable);
}

TEST(ErrorTest, ClientErrorConditionCanceled) {
    EXPECT_EQ(make_error_code(ClientErrorCode::OperationCanceled), ErrorCondition::Canceled);
}

TEST(ErrorTest, ClientErrorConditionNonRetryable) {
    EXPECT_EQ(make_error_code(ClientErrorCode::RequestDisable), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_error_code(ClientErrorCode::OperationNotSupported), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_error_code(ClientErrorCode::ReadDataFail), ErrorCondition::NonRetryable);
}

// --- CredentialsErrorCode ---

TEST(ErrorTest, CredentialsErrorCategory) {
    auto ec = make_error_code(CredentialsErrorCode::Empty);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.credentials"), ec.category().name());
}

TEST(ErrorTest, CredentialsErrorConditionRetryable) {
    EXPECT_EQ(make_error_code(CredentialsErrorCode::FetchError), ErrorCondition::Retryable);
}

TEST(ErrorTest, CredentialsErrorConditionAuthentication) {
    EXPECT_EQ(make_error_code(CredentialsErrorCode::Empty), ErrorCondition::AuthenticationError);
    EXPECT_EQ(make_error_code(CredentialsErrorCode::ProviderNull), ErrorCondition::AuthenticationError);
}

// --- SignerErrorCode ---

TEST(ErrorTest, SignerErrorCategory) {
    auto ec = make_error_code(SignerErrorCode::SignFailed);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.signer"), ec.category().name());
}

TEST(ErrorTest, SignerErrorConditionAuthentication) {
    EXPECT_EQ(make_error_code(SignerErrorCode::SignFailed), ErrorCondition::AuthenticationError);
}

// --- SerdeErrorCode ---

TEST(ErrorTest, SerdeErrorCategory) {
    auto ec = make_error_code(SerdeErrorCode::DeserializationFailed);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.serde"), ec.category().name());
}

TEST(ErrorTest, SerdeErrorConditionNonRetryable) {
    EXPECT_EQ(make_error_code(SerdeErrorCode::DeserializationFailed), ErrorCondition::NonRetryable);
}

// --- ServerError ---

TEST(ErrorTest, ServerErrorCategory) {
    auto ec = make_server_error_code(500);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.server"), ec.category().name());
    EXPECT_EQ(500, ec.value());
}

TEST(ErrorTest, ServerErrorConditionRetryable) {
    EXPECT_EQ(make_server_error_code(500), ErrorCondition::Retryable);
    EXPECT_EQ(make_server_error_code(502), ErrorCondition::Retryable);
    EXPECT_EQ(make_server_error_code(503), ErrorCondition::Retryable);
    EXPECT_EQ(make_server_error_code(401), ErrorCondition::Retryable);
    EXPECT_EQ(make_server_error_code(408), ErrorCondition::Retryable);
    EXPECT_EQ(make_server_error_code(429), ErrorCondition::Retryable);
}

TEST(ErrorTest, ServerErrorConditionNonRetryable) {
    EXPECT_EQ(make_server_error_code(400), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_server_error_code(403), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_server_error_code(404), ErrorCondition::NonRetryable);
}

TEST(ErrorTest, RetryableServerErrorAlwaysRetryable) {
    EXPECT_EQ(make_retryable_server_error_code(403), ErrorCondition::Retryable);
    EXPECT_EQ(make_retryable_server_error_code(404), ErrorCondition::Retryable);
    EXPECT_EQ(make_retryable_server_error_code(400), ErrorCondition::Retryable);
    EXPECT_EQ(make_retryable_server_error_code(500), ErrorCondition::Retryable);
}

TEST(ErrorTest, RetryableServerErrorCategory) {
    auto ec = make_retryable_server_error_code(403);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.server"), ec.category().name());
    EXPECT_EQ(10403, ec.value());
}

TEST(ErrorTest, RetryableServerErrorMessage) {
    auto ec = make_retryable_server_error_code(403);
    EXPECT_EQ("Forbidden", ec.message());
}

// --- TransportErrorCode ---

TEST(ErrorTest, TransportErrorCategory) {
    auto ec = make_error_code(TransportErrorCode::ConnectionFailed);
    EXPECT_TRUE(ec);
    EXPECT_EQ(std::string("oss2.transport"), ec.category().name());
}

TEST(ErrorTest, TransportErrorConditionRetryable) {
    EXPECT_EQ(make_error_code(TransportErrorCode::ConnectionFailed), ErrorCondition::Retryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::DnsError), ErrorCondition::Retryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::Timeout), ErrorCondition::Retryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::SendRecvError), ErrorCondition::Retryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::PartialTransfer), ErrorCondition::Retryable);
}

TEST(ErrorTest, TransportErrorConditionNonRetryable) {
    EXPECT_EQ(make_error_code(TransportErrorCode::SslError), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::NotSupported), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_error_code(TransportErrorCode::Unknown), ErrorCondition::NonRetryable);
}

TEST(ErrorTest, TransportErrorConditionCanceled) {
    EXPECT_EQ(make_error_code(TransportErrorCode::Canceled), ErrorCondition::Canceled);
}

// --- ErrorCondition ---

TEST(ErrorTest, ErrorConditionCategory) {
    auto cond = make_error_condition(ErrorCondition::Retryable);
    EXPECT_EQ(std::string("oss2.condition"), cond.category().name());
}


