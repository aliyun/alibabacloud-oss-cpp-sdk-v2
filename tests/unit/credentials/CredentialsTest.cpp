#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/Credentials.h"

using namespace alibabacloud::oss2;

TEST(CredentialsTest, EmptyCredentials) {
    auto cred = Credentials("", "");
    EXPECT_EQ("", cred.getAccessKeyId());
    EXPECT_EQ("", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
    EXPECT_EQ(false, cred.hasKeys());
    EXPECT_EQ(false, cred.isExpired());
}

TEST(CredentialsTest, NonEmptyCredentials) {
    auto cred = Credentials("ak", "sk");
    EXPECT_EQ("ak", cred.getAccessKeyId());
    EXPECT_EQ("sk", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
    EXPECT_EQ(true, cred.hasKeys());
    EXPECT_EQ(false, cred.isExpired());
}

TEST(CredentialsTest, NonEmptyStsCredentials) {
    auto cred = Credentials("ak", "sk", "token");
    EXPECT_EQ("ak", cred.getAccessKeyId());
    EXPECT_EQ("sk", cred.getAccessKeySecret());
    EXPECT_EQ("token", cred.getSessionToken());
    EXPECT_EQ(true, cred.hasKeys());
    EXPECT_EQ(false, cred.isExpired());
}

TEST(CredentialsTest, DefaultHasNoError) {
    auto cred = Credentials("ak", "sk");
    EXPECT_FALSE(cred.getError().has_value());
}

TEST(CredentialsTest, WithError) {
    auto cred = Credentials::withError("connection timeout");
    EXPECT_TRUE(cred.getError().has_value());
    EXPECT_EQ("connection timeout", cred.getError().value());
    EXPECT_FALSE(cred.hasKeys());
}

TEST(CredentialsTest, WithErrorEmptyKeys) {
    auto cred = Credentials::withError("token expired");
    EXPECT_EQ("", cred.getAccessKeyId());
    EXPECT_EQ("", cred.getAccessKeySecret());
    EXPECT_EQ("", cred.getSessionToken());
}

TEST(CredentialsTest, WithErrorNotRetryable) {
    auto cred = Credentials::withError("bad config");
    EXPECT_FALSE(cred.isErrorRetryable());
}

TEST(CredentialsTest, WithRetryableError) {
    auto cred = Credentials::withRetryableError("network timeout");
    EXPECT_TRUE(cred.getError().has_value());
    EXPECT_EQ("network timeout", cred.getError().value());
    EXPECT_TRUE(cred.isErrorRetryable());
    EXPECT_FALSE(cred.hasKeys());
}