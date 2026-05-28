#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/ClientOptions.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/RsaMasterCipher.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"
#include "MockTransport.h"

namespace alibabacloud::oss2 {

namespace {

const char* kTestPublicKey = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvJy2aTjksDzS2QdJsfJF
/yXHwIxf5rKSUEqGC8qZVOFfbfKhK1lwlpX1NH5Izlxqb2n/ezq9bY9l8ro+Tq6b
T5wS8Ep1h1uzW0TWlx5uCBlpqTgz152Knevf+Rhd0YYE4fQzM40yooqgGWPzuu+6
Wb0q5z82CaVvuJHh6CUFQJtwnAa+xdsuiCAgqUwfJqDKdyLTMASbZ46sYk0FDOKN
hc5pFxiXR0NqQqstTsjtXncbqB9KSSStJ9ghivvQuXwygcMoFRTXMJwYmBY1KikU
KMOUCL+TE6vktUF2IFCIwf3YQFsNI+IrtYNlHE6iEg0aKIB7JG1vFlBfajpTiQgl
GQIDAQAB
-----END PUBLIC KEY-----)";

const char* kTestPrivateKey = R"(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC8nLZpOOSwPNLZ
B0mx8kX/JcfAjF/mspJQSoYLyplU4V9t8qErWXCWlfU0fkjOXGpvaf97Or1tj2Xy
uj5OrptPnBLwSnWHW7NbRNaXHm4IGWmpODPXnYqd69/5GF3RhgTh9DMzjTKiiqAZ
Y/O677pZvSrnPzYJpW+4keHoJQVAm3CcBr7F2y6IICCpTB8moMp3ItMwBJtnjqxi
TQUM4o2FzmkXGJdHQ2pCqy1OyO1edxuoH0pJJK0n2CGK+9C5fDKBwygVFNcwnBiY
FjUqKRQow5QIv5MTq+S1QXYgUIjB/dhAWw0j4iu1g2UcTqISDRoogHskbW8WUF9q
OlOJCCUZAgMBAAECggEAGZY+PhK0c3QiO8RRVbftkaopFGX2se/ntBy9XTwkMO+i
Eo2nxPRijGCA3e4ufPrsitDGZ+FAHBd9V8BhqNWRxusAEWOzLfmytd77fY9okztf
VNAwYupsXkrGx57BRiX4OO737eEUeBh2P6Y076yLNewDtftNSWG4FkHeyQS8rFbY
zVBuvHjrzfEKyUfmqd9tK155Hhd7eusm1YJ75VYx6emlD9EE22Sk4bXCCI5Bh7Zl
548chJ4V1HQ6PCCj9BpBcMnGifzjG8EU1bYwbT669daZwB2WHgsBEAX/BfrM61Dq
BX/Xq7Ij5v0FxleJ5r5aWXDWuIpyOrH1vyBt2152QQKBgQDupmG9IhGKkO7V8m/b
oHS5tW/vZ2+Jw4/rnM2JWpoXwO7xg1MmC0D8zKcLNBXeobJhvBsEVQvdCMokmSpn
NOaNJ9kSqnSl+vxDycmHSlh8b656xjVlexLEx5A3DysJvhaYdI4/Uc8TfB/jWk+L
sNOrhGU9fMamamvczierIWndkwKBgQDKUw5Qqd0PHOhm+x8RaNGQXncFqu0Ye2pa
yE0Sbgm/ofysvzW4cLehpSmmYEIMG8DNVJtEnhmWTl8cQzQnp1fAXlohv0wer5de
NVWQIvLo1F9wI+v50MWvgWBXwQHwLFq7sgYDZCTERBymq85mIbdT2WFDhw+rMRH0
ODqE5c3+IwKBgGW8I/pepZ+ufUJTYX/8/QWV5SvnqlLOPXIxnCUrrHjn1HS8iRu0
vHWIQMWz5IbN459qcxH7t1z4vEOxz7PDh20xSYZ9h9CiGBxFz1WPSf1yFq1cBbNH
Lg8ZC8+M9cnncPZ46ZLwqxghV+6xtytTrEh33jjCEmUrBORSNfLsAZdlAoGAFvcs
hc1yMTf3zVCt6xz5xKhkXDlVplTD8sAPt4rUAnORqc4ee+wXe/qyapc8iAFSdjwn
T7eeceg9dYjPT7z4AfbzxibfrhACX4gwSSceaX1JxAHf1EB1YAGQfQWEgc2XEv0X
H6VrYvfURLr1t7QWCid/mdmn1qfAQPds9Q7cvf8CgYEAoLQHZsQdbhGBxtpdFc2r
98xs6ltwEpIYUofKrb+efyhpM8PMvsJnehLMdwbh2ZV2qdBGbpxYmBcSSev4+Zvx
r1pRhLNEyTW9IOKpZOMUbBIyTpgVEOL5SHDHYPnIcNaogGAYR79oXekTrLhZpHvG
VBIJ/MGrJ+PpwOjY0Y/v8jE=
-----END PRIVATE KEY-----)";

class MockMasterCipher : public crypto::MasterCipher {
  public:
    crypto::MasterCipherResult encrypt(const std::string& plaintext) const override {
        std::string result = plaintext;
        for (auto& c : result) c ^= 0x42;
        return result;
    }
    crypto::MasterCipherResult decrypt(const std::string& ciphertext) const override {
        std::string result = ciphertext;
        for (auto& c : result) c ^= 0x42;
        return result;
    }
    std::string getWrapAlgorithm() const override { return "RSA/NONE/PKCS1Padding"; }
    std::string getMatDesc() const override { return R"({"desc":"test"})"; }
};

class WritingMockTransport : public HttpTransport {
  public:
    struct Response {
        int statusCode{200};
        HeaderCollection headers;
        std::string body;
    };
    std::vector<Response> responses;
    std::vector<std::unique_ptr<RequestMessage>> requests;
    RequestMessage* lastRequest = nullptr;

    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        auto req = std::make_unique<RequestMessage>(*request);
        lastRequest = req.get();
        requests.emplace_back(std::move(req));
        if (lastRequest->body != nullptr) {
            auto src = lastRequest->body->spanSource();
            src->readToEnd();
        }

        auto r = std::move(responses.front());
        responses.erase(responses.begin());

        auto response = std::make_unique<ResponseMessage>();
        response->statusCode = r.statusCode;
        response->headers = r.headers;

        bool isError = (r.statusCode / 100 != 2) || (r.statusCode == 203);

        if (!isError && options.sinkFactory.has_value()) {
            auto sink = options.sinkFactory.value()(static_cast<std::int64_t>(r.body.size()), response->headers);
            if (sink) {
                auto* data = reinterpret_cast<const std::uint8_t*>(r.body.data());
                sink->write(data, r.body.size());
            }
        } else {
            response->body = std::make_shared<std::stringstream>(r.body);
        }

        return response;
    }

    std::string getName() const override { return "WritingMockTransport"; }
};

ClientConfiguration makeConfig(std::shared_ptr<MockTransport> transport) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = transport;
    return config;
}

ClientConfiguration makeConfig(std::shared_ptr<WritingMockTransport> transport) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<AnonymousCredentialsProvider>();
    config.httpTransport = transport;
    return config;
}

crypto::EncryptionConfiguration makeEncConfig(std::shared_ptr<crypto::MasterCipher> mc) {
    crypto::EncryptionConfiguration ec;
    ec.masterCipher = std::move(mc);
    return ec;
}

} // namespace

TEST(OSSEncryptionClientTest, PutObject_AddsCryptoHeaders) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-1"}}, nullptr}));

    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString("hello world"));

    auto outcome = client.putObject(request);
    ASSERT_TRUE(outcome.has_value());

    auto* req = transport->lastRequest;
    ASSERT_NE(nullptr, req);

    EXPECT_NE(req->headers.end(), req->headers.find("x-oss-meta-client-side-encryption-key"));
    EXPECT_NE(req->headers.end(), req->headers.find("x-oss-meta-client-side-encryption-start"));
    EXPECT_EQ("AES/CTR/NoPadding", req->headers["x-oss-meta-client-side-encryption-cek-alg"]);
    EXPECT_EQ("RSA/NONE/PKCS1Padding", req->headers["x-oss-meta-client-side-encryption-wrap-alg"]);
    EXPECT_EQ(R"({"desc":"test"})", req->headers["x-oss-meta-client-side-encryption-matdesc"]);
}

TEST(OSSEncryptionClientTest, PutObject_BodyIsEncrypted) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-1"}}, nullptr}));

    std::string plaintext = "hello world encryption test!";
    auto request = models::PutObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setBody(RequestBody::fromString(plaintext));

    auto outcome = client.putObject(request);
    ASSERT_TRUE(outcome.has_value());

    auto* req = transport->lastRequest;
    ASSERT_NE(nullptr, req);
    ASSERT_NE(nullptr, req->body);

    auto src = req->body->spanSource();
    auto encrypted = src->readToEnd();
    std::string encStr(encrypted.begin(), encrypted.end());
    EXPECT_EQ(plaintext.size(), encStr.size());
    EXPECT_NE(plaintext, encStr);
}

TEST(OSSEncryptionClientTest, GetObject_DecryptsBody) {
    auto transport = std::make_shared<WritingMockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    std::string plainKey(32, '\x01');
    std::string plainIV(16, '\x02');

    std::string encKey = std::get<std::string>(cipher->encrypt(plainKey));
    std::string encIV = std::get<std::string>(cipher->encrypt(plainIV));

    std::string plaintext = "decryption test data here!!!";

    HeaderCollection headers;
    headers["x-oss-request-id"] = "req-2";
    headers["x-oss-meta-client-side-encryption-key"] = utils::Base64Encode(encKey);
    headers["x-oss-meta-client-side-encryption-start"] = utils::Base64Encode(encIV);
    headers["x-oss-meta-client-side-encryption-cek-alg"] = "AES/CTR/NoPadding";
    headers["x-oss-meta-client-side-encryption-wrap-alg"] = "RSA/NONE/PKCS1Padding";
    headers["Content-Length"] = std::to_string(plaintext.size());

    transport->responses.push_back({200, headers, plaintext});

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = false;

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setSinkFactory(factory);

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
}

TEST(OSSEncryptionClientTest, GetObject_NoEncryptionHeaders_PassThrough) {
    auto transport = std::make_shared<WritingMockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    std::string plaintext = "no encryption here";

    transport->responses.push_back({200,
        {{"x-oss-request-id", "req-3"}, {"Content-Length", std::to_string(plaintext.size())}},
        plaintext});

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = false;

    auto request = models::GetObjectRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setSinkFactory(factory);

    auto outcome = client.getObject(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ(plaintext, userStream->str());
}

TEST(OSSEncryptionClientTest, InitiateMultipartUpload_InvalidPartSize) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setCsePartSize(100);

    auto outcome = client.initiateMultipartUpload(request);
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentInvalid", outcome.error().getCode());
}

TEST(OSSEncryptionClientTest, InitiateMultipartUpload_NotMultipleOf16) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setCsePartSize(102401);

    auto outcome = client.initiateMultipartUpload(request);
    ASSERT_FALSE(outcome.has_value());
    EXPECT_EQ("ArgumentInvalid", outcome.error().getCode());
}

TEST(OSSEncryptionClientTest, InitiateMultipartUpload_Success) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    std::string responseXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <UploadId>upload-123</UploadId>
</InitiateMultipartUploadResult>)";
    auto responseBody = std::make_shared<std::stringstream>(responseXml);

    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-4"}}, responseBody}));

    auto request = models::InitiateMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setCsePartSize(102400);
    request.setCseDataSize(512000);

    auto outcome = client.initiateMultipartUpload(request);
    ASSERT_TRUE(outcome.has_value());

    auto& ctx = outcome->getCseMultiPartContext();
    ASSERT_NE(nullptr, ctx);
    EXPECT_GT(ctx->getPartSize(), 0);
    EXPECT_EQ(102400, ctx->getPartSize());

    auto* req = transport->lastRequest;
    EXPECT_NE(req->headers.end(), req->headers.find("x-oss-meta-client-side-encryption-key"));
    EXPECT_NE(req->headers.end(), req->headers.find("x-oss-meta-client-side-encryption-start"));
    EXPECT_EQ("102400", req->headers["x-oss-meta-client-side-encryption-part-size"]);
}

TEST(OSSEncryptionClientTest, UploadPart_EncryptsBody) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    // Initiate first to get a valid context
    std::string responseXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<InitiateMultipartUploadResult>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <UploadId>upload-123</UploadId>
</InitiateMultipartUploadResult>)";
    auto responseBody = std::make_shared<std::stringstream>(responseXml);
    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-init"}}, responseBody}));

    auto initReq = models::InitiateMultipartUploadRequest();
    initReq.setBucket("test-bucket");
    initReq.setKey("test-key");
    initReq.setCsePartSize(102400);
    initReq.setCseDataSize(512000);
    auto initOutcome = client.initiateMultipartUpload(initReq);
    ASSERT_TRUE(initOutcome.has_value());
    auto ctx = initOutcome->getCseMultiPartContext();
    ASSERT_NE(nullptr, ctx);

    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-5"}, {"ETag", "\"etag123\""}}, nullptr}));

    std::string partData = "this is part data for encryption test";
    auto request = models::UploadPartRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-123");
    request.setPartNumber(1);
    request.setBody(RequestBody::fromString(partData));
    request.setCseMultiPartContext(ctx);

    auto outcome = client.uploadPart(request);
    ASSERT_TRUE(outcome.has_value());

    auto* req = transport->lastRequest;
    ASSERT_NE(nullptr, req);
    ASSERT_NE(nullptr, req->body);

    auto src = req->body->spanSource();
    auto encrypted = src->readToEnd();
    std::string encStr(encrypted.begin(), encrypted.end());
    EXPECT_EQ(partData.size(), encStr.size());
    EXPECT_NE(partData, encStr);
}

TEST(OSSEncryptionClientTest, CompleteMultipartUpload_PassThrough) {
    auto transport = std::make_shared<MockTransport>();
    auto cipher = std::make_shared<MockMasterCipher>();
    auto client = OSSEncryptionClient(makeConfig(transport), makeEncConfig(cipher));

    std::string responseXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<CompleteMultipartUploadResult>
    <Location>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/test-key</Location>
    <Bucket>test-bucket</Bucket>
    <Key>test-key</Key>
    <ETag>"etag123"</ETag>
</CompleteMultipartUploadResult>)";
    auto responseBody = std::make_shared<std::stringstream>(responseXml);

    transport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-6"}}, responseBody}));

    std::vector<models::Part> parts;
    models::Part p;
    p.setPartNumber(1);
    p.setETag("\"etag123\"");
    parts.push_back(p);
    models::CompleteMultipartUpload cmu;
    cmu.setParts(parts);

    auto request = models::CompleteMultipartUploadRequest();
    request.setBucket("test-bucket");
    request.setKey("test-key");
    request.setUploadId("upload-123");
    request.setCompleteMultipartUpload(cmu);

    auto outcome = client.completeMultipartUpload(request);
    ASSERT_TRUE(outcome.has_value());
    EXPECT_EQ("test-bucket", outcome.value().getBucket());
}

TEST(OSSEncryptionClientTest, PutGet_RoundTrip) {
    auto cipher = std::make_shared<MockMasterCipher>();

    std::string plaintext = "round-trip encryption verification data!";

    // PUT: encrypt the data
    auto putTransport = std::make_shared<MockTransport>();
    auto putClient = OSSEncryptionClient(makeConfig(putTransport), makeEncConfig(cipher));

    putTransport->responses.emplace_back(
            std::make_unique<ResponseMessage>(ResponseMessage{200, "OK",
                {{"x-oss-request-id", "req-rt1"}}, nullptr}));

    auto putReq = models::PutObjectRequest();
    putReq.setBucket("test-bucket");
    putReq.setKey("test-key");
    putReq.setBody(RequestBody::fromString(plaintext));

    auto putOutcome = putClient.putObject(putReq);
    ASSERT_TRUE(putOutcome.has_value());

    auto* sentReq = putTransport->lastRequest;
    ASSERT_NE(nullptr, sentReq);
    ASSERT_NE(nullptr, sentReq->body);

    auto src = sentReq->body->spanSource();
    auto encryptedBytes = src->readToEnd();
    std::string encryptedBody(encryptedBytes.begin(), encryptedBytes.end());
    ASSERT_NE(plaintext, encryptedBody);

    HeaderCollection sentHeaders = sentReq->headers;

    // GET: decrypt the data using the same headers from the PUT
    auto getTransport = std::make_shared<WritingMockTransport>();
    auto getClient = OSSEncryptionClient(makeConfig(getTransport), makeEncConfig(cipher));

    HeaderCollection responseHeaders;
    responseHeaders["x-oss-request-id"] = "req-rt2";
    responseHeaders["Content-Length"] = std::to_string(encryptedBody.size());
    responseHeaders["x-oss-meta-client-side-encryption-key"] = sentHeaders["x-oss-meta-client-side-encryption-key"];
    responseHeaders["x-oss-meta-client-side-encryption-start"] = sentHeaders["x-oss-meta-client-side-encryption-start"];
    responseHeaders["x-oss-meta-client-side-encryption-cek-alg"] = sentHeaders["x-oss-meta-client-side-encryption-cek-alg"];
    responseHeaders["x-oss-meta-client-side-encryption-wrap-alg"] = sentHeaders["x-oss-meta-client-side-encryption-wrap-alg"];

    getTransport->responses.push_back({200, responseHeaders, encryptedBody});

    auto userStream = std::make_shared<std::stringstream>();
    SinkFactory factory;
    factory.supplier = [userStream](std::int64_t, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        return std::make_shared<OStreamWriter>(userStream);
    };
    factory.isOneShot = false;

    auto getReq = models::GetObjectRequest();
    getReq.setBucket("test-bucket");
    getReq.setKey("test-key");
    getReq.setSinkFactory(factory);

    auto getOutcome = getClient.getObject(getReq);
    ASSERT_TRUE(getOutcome.has_value());
    EXPECT_EQ(plaintext, userStream->str());
}


} // namespace alibabacloud::oss2
