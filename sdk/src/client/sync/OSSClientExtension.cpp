
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "src/internal/sync/ClientImpl.h"

#include <fstream>

namespace alibabacloud {
namespace oss2 {

PutObjectOutcome OSSClient::putObjectFromFile(const models::PutObjectRequest& request,
                                              const std::string& filePath,
                                              const OperationOptions* options) {
    auto req = request;
    req.setBody(RequestBody::FromFile(filePath));
    return putObject(req, options);
}

GetObjectOutcome OSSClient::getObjectToFile(const models::GetObjectRequest& request,
                                            const std::string& filePath,
                                            const OperationOptions* options) {
    const bool crcEnabled = client_->hasFlag(FeatureFlagsType::EnableCRC64CheckDownload) &&
                            request.getRange().empty();

    std::int64_t offset = 0;
    models::GetObjectRequest req = request;

    for (;;) {
        std::shared_ptr<std::ofstream> fileStream = nullptr;
        auto crcObserver = crcEnabled ? std::make_shared<CRC64WriteObserver>() : nullptr;

        SinkFactory factory;
        factory.isOneShot = true;
        factory.supplier = [&fileStream, &filePath, offset, crcObserver](
                std::int64_t /*contentLength*/) -> std::shared_ptr<ByteWriter> {
            if (offset == 0) {
                fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::trunc);
            } else {
                fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::in | std::ios::out);
                fileStream->seekp(offset);
            }
            auto writer = std::make_shared<OStreamWriter>(fileStream);

            if (crcObserver) {
                return std::make_shared<ObservableWriter>(writer, crcObserver);
            }
            return writer;
        };

        if (offset > 0) {
            req.setRange("bytes=" + std::to_string(offset) + "-");
            req.setRangeBehavior("standard");
        }
        req.setSinkFactory(factory);

        auto outcome = getObject(req, options);

        if (outcome.has_value()) {
            if (crcObserver && offset == 0) {
                const auto& serverCrc = outcome.value().getHashCrc64ecma();
                if (!serverCrc.empty()) {
                    uint64_t expected = outcome.value().getHashCrc64ecmaAsUint64();
                    if (expected != crcObserver->crc()) {
                        return makeUnexpected(OperationError(
                                ClientErrorCode::CrcMismatch,
                                {{"Code", "CrcMismatch"},
                                 {"Message", "CRC-64 verification failed"}}));
                    }
                }
            }
            return outcome;
        }

        if (!fileStream) {
            return outcome;
        }

        fileStream->flush();
        if (fileStream->fail()) {
            return outcome;
        }
        std::int64_t newOffset = static_cast<std::int64_t>(fileStream->tellp());

        offset = newOffset;
    }
}

BoolOutcome OSSClient::isObjectExist(const std::string& bucket, const std::string& key,
                                     const OperationOptions* options) {
    auto outcome = getObjectMeta(
            models::GetObjectMetaRequest().setBucket(bucket).setKey(key), options);
    if (outcome.has_value()) {
        return true;
    }
    const auto& e = outcome.error();
    if (e.getCode() == "NoSuchKey" ||
        (e.getStatusCode() == 404 && e.getCode() == "BadErrorResponse")) {
        return false;
    }
    return makeUnexpected(std::move(outcome.error()));
}

BoolOutcome OSSClient::isBucketExist(const std::string& bucket,
                                     const OperationOptions* options) {
    auto outcome = getBucketAcl(
            models::GetBucketAclRequest().setBucket(bucket), options);
    if (outcome.has_value()) {
        return true;
    }
    const auto& e = outcome.error();
    if (e.getCode() == "NoSuchBucket") {
        return false;
    }
    if (e.getStatusCode() > 0) {
        return true;
    }
    return makeUnexpected(std::move(outcome.error()));
}

} // namespace oss2
} // namespace alibabacloud
