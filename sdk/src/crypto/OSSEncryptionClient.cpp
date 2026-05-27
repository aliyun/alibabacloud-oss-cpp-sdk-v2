#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/ContentCipher.h"
#include "AesCtrCipherBuilder.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"
#include "src/utils/Utils.h"

#include <algorithm>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

namespace {

constexpr const char* kCseKeyHeader = "x-oss-meta-client-side-encryption-key";
constexpr const char* kCseStartHeader = "x-oss-meta-client-side-encryption-start";
constexpr const char* kCseCekAlgHeader = "x-oss-meta-client-side-encryption-cek-alg";
constexpr const char* kCseWrapAlgHeader = "x-oss-meta-client-side-encryption-wrap-alg";
constexpr const char* kCseMatDescHeader = "x-oss-meta-client-side-encryption-matdesc";
constexpr const char* kCseDataSizeHeader = "x-oss-meta-client-side-encryption-data-size";
constexpr const char* kCsePartSizeHeader = "x-oss-meta-client-side-encryption-part-size";
constexpr const char* kCseUnencryptedContentLengthHeader = "x-oss-meta-client-side-encryption-unencrypted-content-length";
constexpr const char* kCseUnencryptedContentMD5Header = "x-oss-meta-client-side-encryption-unencrypted-content-md5";

class ErrorWriter final : public ByteWriter {
    std::size_t onWrite(const std::uint8_t*, std::size_t) override {
        return 0;
    }
    int iostate() const override {
        return std::ios_base::badbit;
    }
};

class DiscardWriter final : public ByteWriter {
  public:
    DiscardWriter(std::shared_ptr<ByteWriter> inner, int64_t discardBytes)
        : inner_(std::move(inner)), remaining_(discardBytes) {}

  private:
    std::size_t onWrite(const std::uint8_t* data, std::size_t n) override {
        std::size_t skip = 0;
        if (remaining_ > 0) {
            skip = static_cast<std::size_t>(
                std::min(remaining_, static_cast<int64_t>(n)));
            data += skip;
            n -= skip;
            remaining_ -= static_cast<int64_t>(skip);
        }
        if (n > 0) {
            auto written = inner_->write(data, n);
            return skip + written;
        }
        return skip;
    }

    int iostate() const override { return inner_->state(); }

    std::shared_ptr<ByteWriter> inner_;
    int64_t remaining_;
};

OperationError makeClientError(ClientErrorCode code, const std::string& message) {
    std::string codeStr;
    switch (code) {
        case ClientErrorCode::EncryptionFailure: codeStr = "EncryptionFailure"; break;
        case ClientErrorCode::ArgumentInvalid:   codeStr = "ArgumentInvalid"; break;
        default:                                 codeStr = "ClientError"; break;
    }
    return OperationError(code, {{"Code", codeStr}, {"Message", message}});
}

struct CryptoHeaderOptions {
    std::optional<int64_t> plainContentLen;
    std::optional<int64_t> partSize;
    std::optional<int64_t> dataSize;
};

void addCryptoHeaders(HeaderCollection& meta,
                      const std::unique_ptr<crypto::ContentCipher>& cc,
                      const crypto::CipherMetadata& cm,
                      const CryptoHeaderOptions& opts = {}) {
    auto& cd = cc->getCipherData();
    meta["client-side-encryption-key"] = utils::Base64Encode(cd.encryptedKey);
    meta["client-side-encryption-start"] = utils::Base64Encode(cd.encryptedIV);
    meta["client-side-encryption-cek-alg"] = cm.cekAlgorithm;
    meta["client-side-encryption-wrap-alg"] = cm.wrapAlgorithm;
    if (!cm.matDesc.empty()) {
        meta["client-side-encryption-matdesc"] = cm.matDesc;
    }
    if (opts.plainContentLen.has_value() &&
        cc->getEncryptedLen(*opts.plainContentLen) != *opts.plainContentLen) {
        meta["client-side-encryption-unencrypted-content-length"] =
            std::to_string(*opts.plainContentLen);
    }
    if (opts.partSize.has_value()) {
        meta["client-side-encryption-part-size"] = std::to_string(*opts.partSize);
    }
    if (opts.dataSize.has_value() && *opts.dataSize > 0) {
        meta["client-side-encryption-data-size"] = std::to_string(*opts.dataSize);
    }
}

crypto::Envelope getEnvelopeFromHeaders(const HeaderCollection& headers) {
    crypto::Envelope env;
    auto find = [&headers](const char* key) -> std::string {
        auto it = headers.find(key);
        return it != headers.end() ? it->second : std::string{};
    };
    env.cipherKey = find(kCseKeyHeader);
    env.iv = find(kCseStartHeader);
    env.cekAlg = find(kCseCekAlgHeader);
    env.wrapAlg = find(kCseWrapAlgHeader);
    env.matDesc = find(kCseMatDescHeader);
    env.unencryptedContentLength = find(kCseUnencryptedContentLengthHeader);
    env.unencryptedMD5 = find(kCseUnencryptedContentMD5Header);
    return env;
}

bool hasEnvelope(const crypto::Envelope& env) {
    return !env.iv.empty() && !env.cipherKey.empty() &&
           !env.cekAlg.empty() && !env.wrapAlg.empty();
}

std::optional<OperationError> validateMultiPartContext(
        const std::shared_ptr<crypto::EncryptionMultiPartContext>& ctx) {
    if (!ctx) {
        return makeClientError(
                ClientErrorCode::ArgumentInvalid,
                "CseMultiPartContext is required for encrypted uploadPart");
    }
    int64_t partSize = ctx->getPartSize();
    int64_t alignLen = ctx->getContentCipher().getAlignLen();
    if (partSize <= 0 || partSize % alignLen != 0) {
        return makeClientError(
                ClientErrorCode::ArgumentInvalid,
                "CseMultiPartContext partSize must be a positive multiple of " + std::to_string(alignLen));
    }
    return std::nullopt;
}

std::unique_ptr<crypto::ContentCipherBuilder> resolveCipherBuilder(
        crypto::EncryptionConfiguration& encConfig) {
    if (encConfig.masterCipher) {
        return crypto::CreateAesCtrCipherBuilder(std::move(encConfig.masterCipher));
    }
    return nullptr;
}

} // namespace

struct OSSEncryptionClient::Impl {
    OSSClient client;
    std::unique_ptr<crypto::ContentCipherBuilder> ccBuilder;

    Impl(const ClientConfiguration& config, std::unique_ptr<crypto::ContentCipherBuilder> b)
        : client(config), ccBuilder(std::move(b)) {}
    Impl(const ClientConfiguration& config, ClientOptionsFns& fns, std::unique_ptr<crypto::ContentCipherBuilder> b)
        : client(config, fns), ccBuilder(std::move(b)) {}

    void fillCryptoHeaders(HeaderCollection& meta,
                           const std::unique_ptr<crypto::ContentCipher>& cc,
                           const CryptoHeaderOptions& opts = {}) {
        addCryptoHeaders(meta, cc, ccBuilder->getCipherMetadata(), opts);
    }

    std::shared_ptr<ByteWriter> buildDecryptWriter(
            std::shared_ptr<ByteWriter> writer,
            const HeaderCollection& headers,
            int64_t alignedStart) {
        auto envelope = getEnvelopeFromHeaders(headers);
        if (!hasEnvelope(envelope)) {
            return writer;
        }
        auto cc = ccBuilder->fromEnvelope(envelope);
        if (!cc) {
            return std::make_shared<ErrorWriter>();
        }
        if (alignedStart > 0) {
            cc->seekTo(static_cast<uint64_t>(alignedStart));
        }
        return cc->decryptContent(std::move(writer), 0);
    }
};

OSSEncryptionClient::OSSEncryptionClient(const ClientConfiguration& config,
                                         crypto::EncryptionConfiguration encryptionConfig)
    : impl_(std::make_shared<Impl>(config, resolveCipherBuilder(encryptionConfig))) {}

OSSEncryptionClient::OSSEncryptionClient(const ClientConfiguration& config,
                                         ClientOptionsFns& fns,
                                         crypto::EncryptionConfiguration encryptionConfig)
    : impl_(std::make_shared<Impl>(config, fns, resolveCipherBuilder(encryptionConfig))) {}

OSSEncryptionClient::~OSSEncryptionClient() = default;
OSSEncryptionClient::OSSEncryptionClient(OSSEncryptionClient&&) noexcept = default;
OSSEncryptionClient& OSSEncryptionClient::operator=(OSSEncryptionClient&&) noexcept = default;

OSSClient& OSSEncryptionClient::unwrap() { return impl_->client; }
const OSSClient& OSSEncryptionClient::unwrap() const { return impl_->client; }

PutObjectOutcome OSSEncryptionClient::putObject(const models::PutObjectRequest& request,
                                                 const OperationOptions* options) {
    if (!impl_->ccBuilder) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::ArgumentInvalid, "EncryptionConfiguration has no contentCipherBuilder or masterCipher"));
    }
    auto cc = impl_->ccBuilder->create();
    if (!cc) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::EncryptionFailure, "Failed to create content cipher"));
    }

    std::optional<int64_t> plainLen;
    std::shared_ptr<ByteContent> body;
    if (request.getBody()) {
        plainLen = request.getBody()->length();
        body = cc->encryptContent(request.getBody());
    }

    auto req = request;
    auto meta = req.getMetadata();
    impl_->fillCryptoHeaders(meta, cc, {plainLen, {}, {}});
    req.setMetadata(std::move(meta));
    req.setBody(std::move(body));

    return impl_->client.putObject(req, options);
}

GetObjectOutcome OSSEncryptionClient::getObject(const models::GetObjectRequest& request,
                                                 const OperationOptions* options) {
    if (!impl_->ccBuilder) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::ArgumentInvalid, "EncryptionConfiguration has no contentCipherBuilder or masterCipher"));
    }
    int64_t rangeStart = 0;
    int64_t rangeEnd = -1;
    if (!request.getRange().empty()) {
        std::vector<std::pair<int64_t, int64_t>> ranges;
        if (!utils::ParseRangeHeader(request.getRange(), ranges) ||
            ranges.size() != 1 || ranges[0].first == -1) {
            return makeUnexpected(makeClientError(
                    ClientErrorCode::ArgumentInvalid,
                    "Invalid, multi-range, or suffix-range is not supported for encrypted getObject"));
        }
        rangeStart = ranges[0].first;
        rangeEnd = ranges[0].second;
    }

    int64_t alignLen = impl_->ccBuilder->getAlignLen();
    int64_t alignedStart = (rangeStart / alignLen) * alignLen;
    int64_t discardCount = rangeStart - alignedStart;

    models::GetObjectRequest req = request;
    if (rangeStart > 0 || rangeEnd >= 0) {
        std::string newRange = "bytes=" + std::to_string(alignedStart) + "-";
        if (rangeEnd >= 0) newRange += std::to_string(rangeEnd);
        req.setRange(std::move(newRange));
        req.setRangeBehavior("standard");
    }

    auto userFactory = request.getSinkFactory();
    std::shared_ptr<std::stringstream> defaultStream;
    if (!userFactory.has_value()) {
        defaultStream = std::make_shared<std::stringstream>();
    }

    SinkFactory decryptFactory;
    if (userFactory.has_value()) {
        decryptFactory.isOneShot = userFactory->isOneShot;
    }
    decryptFactory.supplier = [this, userFactory, discardCount, alignedStart, defaultStream](
            std::int64_t size, const HeaderCollection& headers) -> std::shared_ptr<ByteWriter> {
        std::shared_ptr<ByteWriter> writer;
        if (userFactory.has_value()) {
            writer = (*userFactory)(size, headers);
        } else {
            defaultStream->str("");
            defaultStream->clear();
            writer = std::make_shared<OStreamWriter>(defaultStream);
        }

        if (discardCount > 0) {
            writer = std::make_shared<DiscardWriter>(std::move(writer), discardCount);
        }

        return impl_->buildDecryptWriter(std::move(writer), headers, alignedStart);
    };
    req.setSinkFactory(decryptFactory);

    auto outcome = impl_->client.getObject(req, options);
    if (outcome.has_value() && defaultStream) {
        outcome->setBody(defaultStream);
    }
    return outcome;
}

InitiateMultipartUploadOutcome OSSEncryptionClient::initiateMultipartUpload(
        const models::InitiateMultipartUploadRequest& request,
        const OperationOptions* options) {
    if (!impl_->ccBuilder) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::ArgumentInvalid, "EncryptionConfiguration has no contentCipherBuilder or masterCipher"));
    }
    int64_t alignLen = impl_->ccBuilder->getAlignLen();
    auto partSizeOpt = request.getCsePartSize();
    auto dataSizeOpt = request.getCseDataSize();
    if (!partSizeOpt.has_value() || !dataSizeOpt.has_value() ||
        *partSizeOpt % alignLen != 0 || *partSizeOpt < 102400) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::ArgumentInvalid,
                "csePartSize and cseDataSize are required, csePartSize must be a multiple of " + std::to_string(alignLen) + " and >= 102400"));
    }

    auto cc = impl_->ccBuilder->create();
    if (!cc) {
        return makeUnexpected(makeClientError(
                ClientErrorCode::EncryptionFailure, "Failed to create content cipher"));
    }

    auto req = request;
    auto meta = req.getMetadata();
    impl_->fillCryptoHeaders(meta, cc, {{}, *partSizeOpt, *dataSizeOpt});
    req.setMetadata(std::move(meta));

    auto outcome = impl_->client.initiateMultipartUpload(req, options);
    if (outcome.has_value()) {
        auto ctx = crypto::EncryptionMultiPartContext::create(*partSizeOpt, *dataSizeOpt, std::move(cc));
        outcome->setCseMultiPartContext(std::move(ctx));
    }
    return outcome;
}

UploadPartOutcome OSSEncryptionClient::uploadPart(const models::UploadPartRequest& request,
                                                   const OperationOptions* options) {
    auto& ctx = request.getCseMultiPartContext();
    if (auto err = validateMultiPartContext(ctx)) {
        return makeUnexpected(std::move(*err));
    }

    auto partCipher = ctx->getContentCipher().clone();
    uint64_t offset = static_cast<uint64_t>(request.getPartNumber() - 1) * static_cast<uint64_t>(ctx->getPartSize());
    partCipher->seekTo(offset);

    auto req = request;
    if (req.getBody()) {
        req.setBody(partCipher->encryptContent(req.getBody()));
    }

    return impl_->client.uploadPart(req, options);
}

CompleteMultipartUploadOutcome OSSEncryptionClient::completeMultipartUpload(
        const models::CompleteMultipartUploadRequest& request,
        const OperationOptions* options) {
    return impl_->client.completeMultipartUpload(request, options);
}

AbortMultipartUploadOutcome OSSEncryptionClient::abortMultipartUpload(
        const models::AbortMultipartUploadRequest& request,
        const OperationOptions* options) {
    return impl_->client.abortMultipartUpload(request, options);
}

ListPartsOutcome OSSEncryptionClient::listParts(const models::ListPartsRequest& request,
                                                 const OperationOptions* options) {
    return impl_->client.listParts(request, options);
}

} // namespace oss2
} // namespace alibabacloud
