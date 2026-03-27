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