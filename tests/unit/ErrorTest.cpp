#include <gtest/gtest.h>

#include "alibabacloud/oss2/Error.h"

#include <system_error>

using namespace alibabacloud::oss2;

TEST(ErrorTest, MakeErrorCodeNoError) {
    auto ec = make_error_code(SdkErrorCode::NO_ERROR);
    EXPECT_FALSE(ec);
    EXPECT_EQ(0, ec.value());
    EXPECT_EQ(std::string("SdkError"), ec.category().name());
}

TEST(ErrorTest, MakeErrorCodeCrcInconsistent) {
    auto ec = make_error_code(SdkErrorCode::CRC_INCONSISTENT);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::CRC_INCONSISTENT), ec.value());
}

TEST(ErrorTest, MakeErrorCodeRequestDisable) {
    auto ec = make_error_code(SdkErrorCode::REQUEST_DISABLE);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::REQUEST_DISABLE), ec.value());
}

TEST(ErrorTest, MakeErrorCodeNullPointer) {
    auto ec = make_error_code(SdkErrorCode::NULL_POINTER);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::NULL_POINTER), ec.value());
}

TEST(ErrorTest, MakeErrorCodeArgumentInvalid) {
    auto ec = make_error_code(SdkErrorCode::ARGUMENT_INVALID);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::ARGUMENT_INVALID), ec.value());
}

TEST(ErrorTest, MakeErrorCodeArgumentNull) {
    auto ec = make_error_code(SdkErrorCode::ARGUMENT_NULL);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::ARGUMENT_NULL), ec.value());
}

TEST(ErrorTest, MakeErrorCodeEndpointInvalid) {
    auto ec = make_error_code(SdkErrorCode::ENDPOINT_INVALID);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::ENDPOINT_INVALID), ec.value());
}

TEST(ErrorTest, MakeErrorCodeSignError) {
    auto ec = make_error_code(SdkErrorCode::SIGN_ERROR);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::SIGN_ERROR), ec.value());
}

TEST(ErrorTest, MakeErrorCodeCredentialsEmpty) {
    auto ec = make_error_code(SdkErrorCode::CREDENTIALS_EMPTYNULL);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::CREDENTIALS_EMPTYNULL), ec.value());
}

TEST(ErrorTest, MakeErrorCodeCurlyCouldntConnect) {
    auto ec = make_error_code(SdkErrorCode::CURLE_COULDNT_CONNECT);
    EXPECT_TRUE(ec);
    EXPECT_EQ(static_cast<int>(SdkErrorCode::CURLE_COULDNT_CONNECT), ec.value());
}

TEST(ErrorTest, ErrorCodeCategoryName) {
    auto ec = make_error_code(SdkErrorCode::NO_ERROR);
    EXPECT_EQ(std::string("SdkError"), ec.category().name());
}

TEST(ErrorTest, ErrorCodeMessage) {
    auto ec = make_error_code(SdkErrorCode::NO_ERROR);
    EXPECT_EQ(std::string("no error"), ec.message());
}

TEST(ErrorTest, ErrorCodeUnknownMessage) {
    auto ec = make_error_code(static_cast<SdkErrorCode>(999));
    EXPECT_EQ(std::string("unknown sdk error"), ec.message());
}

TEST(ErrorTest, ErrorCodeComparison) {
    auto ec1 = make_error_code(SdkErrorCode::NO_ERROR);
    auto ec2 = make_error_code(SdkErrorCode::CRC_INCONSISTENT);

    EXPECT_EQ(ec1, ec1);
    EXPECT_NE(ec1, ec2);
}

TEST(ErrorTest, ErrorCodeDefaultConstruction) {
    std::error_code ec;
    EXPECT_FALSE(ec);
    EXPECT_EQ(0, ec.value());
}

TEST(ErrorTest, ErrorClassDefaultConstruction) {
    Error error;
    // Note: status_ is not initialized in default constructor
    EXPECT_EQ("", error.code());
    EXPECT_EQ("", error.message());
}

TEST(ErrorTest, ErrorClassWithValues) {
    Error error("AccessDenied", "Access denied to resource");
    EXPECT_EQ(0, error.status());
    EXPECT_EQ("AccessDenied", error.code());
    EXPECT_EQ("Access denied to resource", error.message());
}
