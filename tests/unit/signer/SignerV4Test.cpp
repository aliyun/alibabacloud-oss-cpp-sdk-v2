#include <gtest/gtest.h>

#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/signer/SignerV4.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"
#include "src/utils/Utils.h"


#include <iostream>

using namespace alibabacloud::oss2;

TEST(SignerV4Test, AuthHeader) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "text/plain"},
                                          {"x-oss-content-sha256", "UNSIGNED-PAYLOAD"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702743657;

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    std::string expectedAuth =
            "OSS4-HMAC-SHA256 "
            "Credential=ak/20231216/cn-hangzhou/oss/"
            "aliyun_v4_request,Signature=e21d18daa82167720f9b1047ae7e7f1ce7cb77a31e8203a7d5f4624fa0284afe";
    EXPECT_EQ(expectedAuth, request.headers.at("Authorization"));
    EXPECT_EQ("20231216T162057Z", request.headers.at("x-oss-date"));
}


TEST(SignerV4Test, AuthHeaderWithToken) {
    auto provider = StaticCredentialsProvider("ak", "sk", "token");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "text/plain"},
                                          {"x-oss-content-sha256", "UNSIGNED-PAYLOAD"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702784856;

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    std::string expectedAuth =
            "OSS4-HMAC-SHA256 "
            "Credential=ak/20231217/cn-hangzhou/oss/"
            "aliyun_v4_request,Signature=b94a3f999cf85bcdc00d332fbd3734ba03e48382c36fa4d5af5df817395bd9ea";
    EXPECT_EQ(expectedAuth, request.headers.at("Authorization"));
    EXPECT_EQ("20231217T034736Z", request.headers.at("x-oss-date"));
    EXPECT_EQ("token", request.headers.at("x-oss-security-token"));
}


TEST(SignerV4Test, AuthHeaderWithAdditionalHeaders) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "text/plain"},
                                          {"x-oss-content-sha256", "UNSIGNED-PAYLOAD"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702747512;
    context.additionalHeaders = {"ZAbc", "abc"};

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    std::string expectedAuth =
            "OSS4-HMAC-SHA256 "
            "Credential=ak/20231216/cn-hangzhou/oss/"
            "aliyun_v4_request,AdditionalHeaders=abc;zabc,"
            "Signature=4a4183c187c07c8947db7620deb0a6b38d9fbdd34187b6dbaccb316fa251212f";
    EXPECT_EQ(expectedAuth, request.headers.at("Authorization"));
    EXPECT_EQ("20231216T172512Z", request.headers.at("x-oss-date"));
}


TEST(SignerV4Test, AuthHeaderWithAdditionalHeadersWitdhDefault) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "text/plain"},
                                          {"x-oss-content-sha256", "UNSIGNED-PAYLOAD"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702747512;
    context.additionalHeaders = {"x-oss-no-exist", "ZAbc", "x-oss-head1", "abc"};

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    std::string expectedAuth =
            "OSS4-HMAC-SHA256 "
            "Credential=ak/20231216/cn-hangzhou/oss/"
            "aliyun_v4_request,AdditionalHeaders=abc;zabc,"
            "Signature=4a4183c187c07c8947db7620deb0a6b38d9fbdd34187b6dbaccb316fa251212f";
    EXPECT_EQ(expectedAuth, request.headers.at("Authorization"));
    EXPECT_EQ("20231216T172512Z", request.headers.at("x-oss-date"));
}


TEST(SignerV4Test, AuthHeaderComplex) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-meta-zzz", "value1"},
                                          {"x-oss-meta-aaa", "value2"},
                                          {"x-oss-meta-123", "value3"},
                                          {"x-oss-meta-abc123", "value4"},
                                          {"x-oss-meta-abc-123", "value5"},
                                          {"x-oss-meta-abc_123", "value6"},
                                          {"x-oss-meta-ABC", "value7"},
                                          {"content-type", "application/json"},
                                          {"x-oss-date", "20250814T080624Z"},
                                          {"content-md5", "md5hash"},
                                          {"x-oss-content-sha256", "UNSIGNED-PAYLOAD"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702743657;

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    std::string expectedAuth =
            "OSS4-HMAC-SHA256 "
            "Credential=ak/20231216/cn-hangzhou/oss/"
            "aliyun_v4_request,"
            "Signature=3e9a6ebd7789767059589cc62116d9e4ebc4787e11b937f1683d0f344cf2693e";
    EXPECT_EQ(expectedAuth, request.headers.at("Authorization"));
    EXPECT_EQ("20231216T162057Z", request.headers.at("x-oss-date"));
}


TEST(SignerV4Test, AuthHeaderReSign) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    HeaderCollection headers{
            {"x-oss-head1", "value"},
            {"content-type", "text/plain"},
    };

    auto makeContext = [&cred](RequestMessage* request, std::time_t signTime) {
        auto context = SigningContext();
        context.bucket = "bucket";
        context.key = "1.txt";
        context.request = request;
        context.credentials = cred;
        context.product = "oss";
        context.region = "cn-hangzhou";
        context.signTimeInEpoch = signTime;
        return context;
    };

    auto signer = SignerV4();

    auto request = RequestMessage{"PUT", "http://bucket.oss-cn-hangzhou.aliyuncs.com/1.txt", headers};
    auto context = makeContext(&request, 1702743657);
    EXPECT_TRUE(signer.sign(context));
    auto firstAuth = request.headers.at("Authorization");
    EXPECT_EQ("20231216T162057Z", request.headers.at("x-oss-date"));

    // the retryer resets the sign time, then the same request object is signed again
    context.signTimeInEpoch = 1702743757;
    EXPECT_TRUE(signer.sign(context));
    EXPECT_EQ("20231216T162237Z", request.headers.at("x-oss-date"));
    EXPECT_NE(firstAuth, request.headers.at("Authorization"));

    // re-signing must yield the same result as signing a fresh request at the new time
    auto expectedRequest = RequestMessage{"PUT", "http://bucket.oss-cn-hangzhou.aliyuncs.com/1.txt", headers};
    auto expectedContext = makeContext(&expectedRequest, 1702743757);
    EXPECT_TRUE(signer.sign(expectedContext));
    EXPECT_EQ(expectedRequest.headers.at("Authorization"), request.headers.at("Authorization"));
    EXPECT_EQ(expectedRequest.headers.at("Date"), request.headers.at("Date"));
}


TEST(SignerV4Test, AuthHeaderReSignWithRefreshedToken) {
    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com/1.txt",
                                  {
                                          {"content-type", "text/plain"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1.txt";
    context.request = &request;
    context.credentials = StaticCredentialsProvider("ak", "sk", "token1").getCredentials();
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702743657;

    auto signer = SignerV4();
    EXPECT_TRUE(signer.sign(context));
    EXPECT_EQ("token1", request.headers.at("x-oss-security-token"));

    context.credentials = StaticCredentialsProvider("ak", "sk", "token2").getCredentials();
    EXPECT_TRUE(signer.sign(context));
    EXPECT_EQ("token2", request.headers.at("x-oss-security-token"));
}


TEST(SignerV4Test, AuthQuery) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "application/octet-stream"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702781677;
    context.expirationInEpoch = 1702782276;
    context.authMethodQuery = true;

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto encodedParameters = utils::ToEncodedParameters(request.uri);

    EXPECT_EQ("OSS4-HMAC-SHA256", encodedParameters.at("x-oss-signature-version"));
    EXPECT_EQ("599", encodedParameters.at("x-oss-expires"));
    EXPECT_EQ("ak%2F20231217%2Fcn-hangzhou%2Foss%2Faliyun_v4_request", encodedParameters.at("x-oss-credential"));
    EXPECT_EQ("a39966c61718be0d5b14e668088b3fa07601033f6518ac7b523100014269c0fe",
              encodedParameters.at("x-oss-signature"));
    EXPECT_EQ(encodedParameters.end(), encodedParameters.find("x-oss-additional-headers"));
    std::string expect = "http://bucket.oss-cn-hangzhou.aliyuncs.com/1234%2B-/123/1.txt";
    EXPECT_EQ(expect, request.uri.substr(0, expect.size()));
}


TEST(SignerV4Test, AuthQueryWithToken) {
    auto provider = StaticCredentialsProvider("ak", "sk", "token");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "application/octet-stream"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702785388;
    context.expirationInEpoch = 1702785987;
    context.authMethodQuery = true;

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto encodedParameters = utils::ToEncodedParameters(request.uri);

    EXPECT_EQ("OSS4-HMAC-SHA256", encodedParameters.at("x-oss-signature-version"));
    EXPECT_EQ("token", encodedParameters.at("x-oss-security-token"));
    EXPECT_EQ("20231217T035628Z", encodedParameters.at("x-oss-date"));
    EXPECT_EQ("599", encodedParameters.at("x-oss-expires"));
    EXPECT_EQ("ak%2F20231217%2Fcn-hangzhou%2Foss%2Faliyun_v4_request", encodedParameters.at("x-oss-credential"));
    EXPECT_EQ("3817ac9d206cd6dfc90f1c09c00be45005602e55898f26f5ddb06d7892e1f8b5",
              encodedParameters.at("x-oss-signature"));
    EXPECT_EQ(encodedParameters.end(), encodedParameters.find("x-oss-additional-headers"));
    std::string expect = "http://bucket.oss-cn-hangzhou.aliyuncs.com/1234%2B-/123/1.txt";
    EXPECT_EQ(expect, request.uri.substr(0, expect.size()));
}

TEST(SignerV4Test, AuthQueryWithAdditionalHeaders) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "application/octet-stream"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702783809;
    context.expirationInEpoch = 1702784408;
    context.authMethodQuery = true;
    context.additionalHeaders = {"ZAbc", "abc"};

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto encodedParameters = utils::ToEncodedParameters(request.uri);

    EXPECT_EQ("OSS4-HMAC-SHA256", encodedParameters.at("x-oss-signature-version"));
    EXPECT_EQ("20231217T033009Z", encodedParameters.at("x-oss-date"));
    EXPECT_EQ("599", encodedParameters.at("x-oss-expires"));
    EXPECT_EQ("ak%2F20231217%2Fcn-hangzhou%2Foss%2Faliyun_v4_request", encodedParameters.at("x-oss-credential"));
    EXPECT_EQ("6bd984bfe531afb6db1f7550983a741b103a8c58e5e14f83ea474c2322dfa2b7",
              encodedParameters.at("x-oss-signature"));
    EXPECT_EQ("abc%3Bzabc", encodedParameters.at("x-oss-additional-headers"));
    std::string expect = "http://bucket.oss-cn-hangzhou.aliyuncs.com/1234%2B-/123/1.txt";
    EXPECT_EQ(expect, request.uri.substr(0, expect.size()));
}

TEST(SignerV4Test, AuthQueryWithAdditionalHeadersWitdhDefault) {
    auto provider = StaticCredentialsProvider("ak", "sk");
    auto cred = provider.getCredentials();

    auto request = RequestMessage{"PUT",
                                  "http://bucket.oss-cn-hangzhou.aliyuncs.com",
                                  {
                                          {"x-oss-head1", "value"},
                                          {"abc", "value"},
                                          {"ZAbc", "value"},
                                          {"XYZ", "value"},
                                          {"content-type", "application/octet-stream"},
                                  }};

    auto context = SigningContext();
    context.bucket = "bucket";
    context.key = "1234+-/123/1.txt";
    context.request = &request;
    context.credentials = cred;
    context.product = "oss";
    context.region = "cn-hangzhou";
    context.signTimeInEpoch = 1702783809;
    context.expirationInEpoch = 1702784408;
    context.authMethodQuery = true;
    context.additionalHeaders = {"x-oss-no-exist", "ZAbc", "x-oss-head1", "abc"};

    auto parameters = ParameterCollection{
            {"param1", "value1"}, {"+param1", "value3"}, {"|param1", "value4"},
            {"+param2", ""},      {"|param2", ""},       {"param2", ""},
    };

    auto query = utils::ToQueryString(parameters);
    request.uri = request.uri + "/" + utils::UrlEncodePath(context.key) + "?" + query;

    auto signer = SignerV4();
    auto result = signer.sign(context);
    EXPECT_TRUE(result);

    auto encodedParameters = utils::ToEncodedParameters(request.uri);

    EXPECT_EQ("OSS4-HMAC-SHA256", encodedParameters.at("x-oss-signature-version"));
    EXPECT_EQ("20231217T033009Z", encodedParameters.at("x-oss-date"));
    EXPECT_EQ("599", encodedParameters.at("x-oss-expires"));
    EXPECT_EQ("ak%2F20231217%2Fcn-hangzhou%2Foss%2Faliyun_v4_request", encodedParameters.at("x-oss-credential"));
    EXPECT_EQ("6bd984bfe531afb6db1f7550983a741b103a8c58e5e14f83ea474c2322dfa2b7",
              encodedParameters.at("x-oss-signature"));
    EXPECT_EQ("abc%3Bzabc", encodedParameters.at("x-oss-additional-headers"));
    std::string expect = "http://bucket.oss-cn-hangzhou.aliyuncs.com/1234%2B-/123/1.txt";
    EXPECT_EQ(expect, request.uri.substr(0, expect.size()));
}
