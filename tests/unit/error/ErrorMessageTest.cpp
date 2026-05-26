#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"

using namespace alibabacloud::oss2;

// --- ErrorCondition message() ---

TEST(ErrorMessageTest, ErrorConditionMessages) {
    EXPECT_EQ("retryable error", make_error_condition(ErrorCondition::Retryable).message());
    EXPECT_EQ("non-retryable error", make_error_condition(ErrorCondition::NonRetryable).message());
    EXPECT_EQ("operation canceled", make_error_condition(ErrorCondition::Canceled).message());
    EXPECT_EQ("invalid argument", make_error_condition(ErrorCondition::InvalidArgument).message());
    EXPECT_EQ("authentication error", make_error_condition(ErrorCondition::AuthenticationError).message());
    // default branch
    auto cond = std::error_condition(99, make_error_condition(ErrorCondition::Retryable).category());
    EXPECT_EQ("unknown condition", cond.message());
}

// --- ClientErrorCode message() ---

TEST(ErrorMessageTest, ClientErrorMessages) {
    EXPECT_EQ("invalid argument", make_error_code(ClientErrorCode::ArgumentInvalid).message());
    EXPECT_EQ("required argument is missing", make_error_code(ClientErrorCode::ArgumentRequired).message());
    EXPECT_EQ("endpoint is invalid", make_error_code(ClientErrorCode::EndpointInvalid).message());
    EXPECT_EQ("endpoint region is null", make_error_code(ClientErrorCode::EndpointRegionNull).message());
    EXPECT_EQ("bucket name is invalid", make_error_code(ClientErrorCode::BucketNameInvalid).message());
    EXPECT_EQ("object name is invalid", make_error_code(ClientErrorCode::ObjectNameInvalid).message());
    EXPECT_EQ("CRC mismatch", make_error_code(ClientErrorCode::CrcMismatch).message());
    EXPECT_EQ("request is disabled", make_error_code(ClientErrorCode::RequestDisable).message());
    EXPECT_EQ("operation canceled", make_error_code(ClientErrorCode::OperationCanceled).message());
    EXPECT_EQ("operation not supported", make_error_code(ClientErrorCode::OperationNotSupported).message());
    EXPECT_EQ("request method is empty", make_error_code(ClientErrorCode::RequestMethodEmpty).message());
    EXPECT_EQ("failed to read data", make_error_code(ClientErrorCode::ReadDataFail).message());
    // default branch
    auto ec = std::error_code(99, make_error_code(ClientErrorCode::ArgumentInvalid).category());
    EXPECT_EQ("unknown client error", ec.message());
}

TEST(ErrorMessageTest, ClientErrorEquivalentNonMatching) {
    // Test the "return false" path - condition that doesn't match any
    auto ec = make_error_code(ClientErrorCode::ArgumentInvalid);
    EXPECT_FALSE(ec == make_error_condition(ErrorCondition::AuthenticationError));

    // EndpointRegionNull is InvalidArgument
    auto ec2 = make_error_code(ClientErrorCode::EndpointRegionNull);
    EXPECT_EQ(ec2, ErrorCondition::InvalidArgument);
    EXPECT_NE(ec2, ErrorCondition::Retryable);
    EXPECT_NE(ec2, ErrorCondition::Canceled);
    EXPECT_NE(ec2, ErrorCondition::NonRetryable);
}

// --- CredentialsErrorCode message() ---

TEST(ErrorMessageTest, CredentialsErrorMessages) {
    EXPECT_EQ("credentials are empty", make_error_code(CredentialsErrorCode::Empty).message());
    EXPECT_EQ("failed to fetch credentials", make_error_code(CredentialsErrorCode::FetchError).message());
    EXPECT_EQ("credentials provider is null", make_error_code(CredentialsErrorCode::ProviderNull).message());
    // default branch
    auto ec = std::error_code(99, make_error_code(CredentialsErrorCode::Empty).category());
    EXPECT_EQ("unknown credentials error", ec.message());
}

TEST(ErrorMessageTest, CredentialsErrorEquivalentNonMatching) {
    auto ec = make_error_code(CredentialsErrorCode::Empty);
    EXPECT_NE(ec, ErrorCondition::Retryable);
    EXPECT_NE(ec, ErrorCondition::InvalidArgument);
    EXPECT_NE(ec, ErrorCondition::Canceled);
    EXPECT_NE(ec, ErrorCondition::NonRetryable);
}

// --- SignerErrorCode message() ---

TEST(ErrorMessageTest, SignerErrorMessages) {
    EXPECT_EQ("signing failed", make_error_code(SignerErrorCode::SignFailed).message());
    // default branch
    auto ec = std::error_code(99, make_error_code(SignerErrorCode::SignFailed).category());
    EXPECT_EQ("unknown signer error", ec.message());
}

TEST(ErrorMessageTest, SignerErrorEquivalentNonMatching) {
    auto ec = make_error_code(SignerErrorCode::SignFailed);
    EXPECT_NE(ec, ErrorCondition::Retryable);
    EXPECT_NE(ec, ErrorCondition::InvalidArgument);
    EXPECT_NE(ec, ErrorCondition::Canceled);
    EXPECT_NE(ec, ErrorCondition::NonRetryable);
}

// --- SerdeErrorCode message() ---

TEST(ErrorMessageTest, SerdeErrorMessages) {
    EXPECT_EQ("deserialization failed", make_error_code(SerdeErrorCode::DeserializationFailed).message());
    // default branch
    auto ec = std::error_code(99, make_error_code(SerdeErrorCode::DeserializationFailed).category());
    EXPECT_EQ("unknown serde error", ec.message());
}

TEST(ErrorMessageTest, SerdeErrorEquivalentNonMatching) {
    auto ec = make_error_code(SerdeErrorCode::DeserializationFailed);
    EXPECT_NE(ec, ErrorCondition::Retryable);
    EXPECT_NE(ec, ErrorCondition::InvalidArgument);
    EXPECT_NE(ec, ErrorCondition::Canceled);
    EXPECT_NE(ec, ErrorCondition::AuthenticationError);
}

// --- TransportErrorCode message() ---

TEST(ErrorMessageTest, TransportErrorMessages) {
    EXPECT_EQ("connection failed", make_error_code(TransportErrorCode::ConnectionFailed).message());
    EXPECT_EQ("DNS resolution failed", make_error_code(TransportErrorCode::DnsError).message());
    EXPECT_EQ("SSL/TLS error", make_error_code(TransportErrorCode::SslError).message());
    EXPECT_EQ("operation timed out", make_error_code(TransportErrorCode::Timeout).message());
    EXPECT_EQ("send/receive error", make_error_code(TransportErrorCode::SendRecvError).message());
    EXPECT_EQ("partial transfer", make_error_code(TransportErrorCode::PartialTransfer).message());
    EXPECT_EQ("operation canceled", make_error_code(TransportErrorCode::Canceled).message());
    EXPECT_EQ("operation not supported", make_error_code(TransportErrorCode::NotSupported).message());
    EXPECT_EQ("unknown transport error", make_error_code(TransportErrorCode::Unknown).message());
    // default branch
    auto ec = std::error_code(99, make_error_code(TransportErrorCode::ConnectionFailed).category());
    EXPECT_EQ("unknown transport error", ec.message());
}

TEST(ErrorMessageTest, TransportErrorEquivalentNonMatching) {
    auto ec = make_error_code(TransportErrorCode::ConnectionFailed);
    EXPECT_NE(ec, ErrorCondition::NonRetryable);
    EXPECT_NE(ec, ErrorCondition::Canceled);
    EXPECT_NE(ec, ErrorCondition::AuthenticationError);
    EXPECT_NE(ec, ErrorCondition::InvalidArgument);

    // SslError is NonRetryable but not Retryable/Canceled
    auto ec2 = make_error_code(TransportErrorCode::SslError);
    EXPECT_NE(ec2, ErrorCondition::Retryable);
    EXPECT_NE(ec2, ErrorCondition::Canceled);
}

// --- ServerError message() ---

TEST(ErrorMessageTest, ServerErrorMessages) {
    EXPECT_EQ("Bad Request", make_server_error_code(400).message());
    EXPECT_EQ("Unauthorized", make_server_error_code(401).message());
    EXPECT_EQ("Forbidden", make_server_error_code(403).message());
    EXPECT_EQ("Not Found", make_server_error_code(404).message());
    EXPECT_EQ("Method Not Allowed", make_server_error_code(405).message());
    EXPECT_EQ("Request Timeout", make_server_error_code(408).message());
    EXPECT_EQ("Conflict", make_server_error_code(409).message());
    EXPECT_EQ("Too Many Requests", make_server_error_code(429).message());
    EXPECT_EQ("Internal Server Error", make_server_error_code(500).message());
    EXPECT_EQ("Bad Gateway", make_server_error_code(502).message());
    EXPECT_EQ("Service Unavailable", make_server_error_code(503).message());
    EXPECT_EQ("Gateway Timeout", make_server_error_code(504).message());
    // generic server error
    EXPECT_EQ("Server Error", make_server_error_code(505).message());
    EXPECT_EQ("Server Error", make_server_error_code(599).message());
    // generic client error
    EXPECT_EQ("Client Error", make_server_error_code(410).message());
    EXPECT_EQ("Client Error", make_server_error_code(499).message());
    // non-4xx/5xx
    EXPECT_EQ("HTTP 200", make_server_error_code(200).message());
    EXPECT_EQ("HTTP 301", make_server_error_code(301).message());
    // retryable server error code message (ev >= 10000)
    EXPECT_EQ("Forbidden", make_retryable_server_error_code(403).message());
    EXPECT_EQ("Internal Server Error", make_retryable_server_error_code(500).message());
}

TEST(ErrorMessageTest, ServerErrorEquivalentEdgeCases) {
    // Non-retryable 4xx codes
    EXPECT_EQ(make_server_error_code(405), ErrorCondition::NonRetryable);
    EXPECT_EQ(make_server_error_code(409), ErrorCondition::NonRetryable);

    // Retryable server error (code >= 10000) always retryable
    EXPECT_EQ(make_retryable_server_error_code(200), ErrorCondition::Retryable);

    // Test non-matching conditions
    EXPECT_NE(make_server_error_code(500), ErrorCondition::Canceled);
    EXPECT_NE(make_server_error_code(500), ErrorCondition::AuthenticationError);
    EXPECT_NE(make_server_error_code(500), ErrorCondition::InvalidArgument);

    // 4xx non-retryable codes don't match Retryable
    EXPECT_NE(make_server_error_code(403), ErrorCondition::Retryable);

    // Test server error equivalent returns false for unmatched condition
    EXPECT_NE(make_server_error_code(200), ErrorCondition::NonRetryable);
    EXPECT_NE(make_server_error_code(200), ErrorCondition::Retryable);
}
