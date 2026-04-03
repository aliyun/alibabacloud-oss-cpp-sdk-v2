#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/signer/SignerV1.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/utils/Utils.h"


#include <iostream>

using namespace alibabacloud::oss2;

TEST(SignerV1Test, AuthHeader1) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://examplebucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"Content-MD5", "eB5eJF1ptWaXm4bijSPyxw=="},
                                          {"Content-Type", "text/html"},
                                          {"x-oss-meta-author", "alice"},
                                          {"x-oss-meta-magic", "abracadabra"},
                                          {"x-oss-date", "Wed, 28 Dec 2022 10:27:41 GMT"},
                                  }};

    auto context = SigningContext();
    context.bucket = "examplebucket";
    context.key = "nelson";
    context.request = &request;
    context.credentials = cred;
    context.signTimeInEpoch = 1702743657;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    EXPECT_EQ("PUT\neB5eJF1ptWaXm4bijSPyxw==\ntext/html\nWed, 28 Dec 2022 10:27:41 GMT\nx-oss-date:Wed, 28 Dec 2022 10:27:41 GMT\nx-oss-meta-author:alice\nx-oss-meta-magic:abracadabra\n/examplebucket/nelson",
              context.stringToSign);
    EXPECT_EQ("OSS ak:kSHKmLxlyEAKtZPkJhG9bZb5k7M=", request.headers.at("Authorization"));
}

TEST(SignerV1Test, AuthHeader2) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://examplebucket.oss-cn-hangzhou.aliyuncs.com/?acl",
                                  {
                                          {"Content-MD5", "eB5eJF1ptWaXm4bijSPyxw=="},
                                          {"Content-Type", "text/html"},
                                          {"x-oss-meta-author", "alice"},
                                          {"x-oss-meta-magic", "abracadabra"},
                                          {"x-oss-date", "Wed, 28 Dec 2022 10:27:41 GMT"},
                                  }};

    auto context = SigningContext();
    context.bucket = "examplebucket";
    context.key = "nelson";
    context.request = &request;
    context.credentials = cred;
    context.signTimeInEpoch = 1702743657;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    EXPECT_EQ("PUT\neB5eJF1ptWaXm4bijSPyxw==\ntext/html\nWed, 28 Dec 2022 10:27:41 GMT\nx-oss-date:Wed, 28 Dec 2022 10:27:41 GMT\nx-oss-meta-author:alice\nx-oss-meta-magic:abracadabra\n/examplebucket/nelson?acl",
              context.stringToSign);
    EXPECT_EQ("OSS ak:/afkugFbmWDQ967j1vr6zygBLQk=", request.headers.at("Authorization"));
}

TEST(SignerV1Test, AuthHeader3) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"GET",
                                  "http://examplebucket.oss-cn-hangzhou.aliyuncs.com/?resourceGroup&non-resousce=null",
                                  {
                                          {"x-oss-date", "Wed, 28 Dec 2022 10:27:41 GMT"},
                                  }};

    auto context = SigningContext();
    context.bucket = "examplebucket";
    context.request = &request;
    context.credentials = cred;
    context.signTimeInEpoch = 1702743657;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    EXPECT_EQ("GET\n\n\nWed, 28 Dec 2022 10:27:41 GMT\nx-oss-date:Wed, 28 Dec 2022 10:27:41 GMT\n/examplebucket/?resourceGroup",
              context.stringToSign);
    EXPECT_EQ("OSS ak:vkQmfuUDyi1uDi3bKt67oemssIs=", request.headers.at("Authorization"));
}

TEST(SignerV1Test, AuthHeader4) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"GET",
                                  "http://examplebucket.oss-cn-hangzhou.aliyuncs.com/?resourceGroup&acl",
                                  {
                                          {"x-oss-date", "Wed, 28 Dec 2022 10:27:41 GMT"},
                                  }};

    auto context = SigningContext();
    context.bucket = "examplebucket";
    context.request = &request;
    context.credentials = cred;
    context.signTimeInEpoch = 1702743657;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    EXPECT_EQ("GET\n\n\nWed, 28 Dec 2022 10:27:41 GMT\nx-oss-date:Wed, 28 Dec 2022 10:27:41 GMT\n/examplebucket/?acl&resourceGroup",
              context.stringToSign);
    EXPECT_EQ("OSS ak:x3E5TgOvl/i7PN618s5mEvpJDYk=", request.headers.at("Authorization"));
}

TEST(SignerV1Test, AuthQuery) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"GET",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com/key?versionId=versionId",
                                  {}};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "key";
    context.request = &request;
    context.credentials = cred;
    context.expirationInEpoch = 1699807420;
    context.authMethodQuery = true;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto queries = utils::ToEncodedParameters(request.uri);

    EXPECT_EQ("versionId", queries.at("versionId"));
    EXPECT_NE(queries.end(), queries.find("Expires"));
    EXPECT_EQ("ak", queries.at("OSSAccessKeyId"));
    EXPECT_EQ("dcLTea%2BYh9ApirQ8o8dOPqtvJXQ%3D", queries.at("Signature"));
}

TEST(SignerV1Test, AuthQueryWithToken) {
    auto provider = StaticCredentialsProvider("ak", "sk", "attachment; /file/name==example.txt++");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"GET",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com/key+123?versionId=versionId",
                                  {}};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "key+123";
    context.request = &request;
    context.credentials = cred;
    context.expirationInEpoch = 1699808204;
    context.authMethodQuery = true;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto queries = utils::ToEncodedParameters(request.uri);

    EXPECT_NE(queries.end(), queries.find("Expires"));
    EXPECT_EQ("ak", queries.at("OSSAccessKeyId"));
    EXPECT_EQ("attachment%3B%20%2Ffile%2Fname%3D%3Dexample.txt%2B%2B", queries.at("security-token"));
    EXPECT_EQ("su58IVk06Q73DHwcMsXft%2FRTZ98%3D", queries.at("Signature"));
}

TEST(SignerV1Test, AuthQueryWithAdditionalParams) {
    auto provider = StaticCredentialsProvider("ak", "sk", "token");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"GET",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com/key?versionId=versionId&param1=value1&%2Bparam1=value3&%7Cparam1=value4&%2Bparam2=&%7Cparam2=&param2=&response-content-disposition=attachment%3B%20filename%3Dexample.txt",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "application/octet-stream"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "key";
    context.request = &request;
    context.credentials = cred;
    context.expirationInEpoch = 1699808204;
    context.authMethodQuery = true;

    auto signer = SignerV1();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto queries = utils::ToEncodedParameters(request.uri);

    EXPECT_NE(queries.end(), queries.find("Expires"));
    EXPECT_EQ("ak", queries.at("OSSAccessKeyId"));
    EXPECT_EQ("attachment%3B%20filename%3Dexample.txt", queries.at("response-content-disposition"));
    EXPECT_EQ("token", queries.at("security-token"));
    EXPECT_EQ("VmWfLWfxbR3MSFvUx5%2BnyQhCa3g%3D", queries.at("Signature"));
}
