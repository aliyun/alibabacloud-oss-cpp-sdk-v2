
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/utils/Utils.h"

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
    std::int64_t rangeStart = 0;
    std::int64_t rangeEnd = -1;
    if (!request.getRange().empty()) {
        std::vector<std::pair<std::int64_t, std::int64_t>> ranges;
        if (!utils::ParseRangeHeader(request.getRange(), ranges) ||
            ranges.size() != 1 || ranges[0].first == -1) {
            return makeUnexpected(OperationError(
                    ClientErrorCode::ArgumentInvalid,
                    {{"Code", "ArgumentInvalid"},
                     {"Message", "Invalid, multi-range, or suffix-range is not supported for getObjectToFile"}}));
        }
        rangeStart = ranges[0].first;
        rangeEnd = ranges[0].second;
    }

    bool isFullDownload = (rangeStart == 0 && rangeEnd == -1);
    std::shared_ptr<CRC64WriteObserver> crcObserver = nullptr;
    if (client_->hasFlag(FeatureFlagsType::EnableCRC64CheckDownload) && isFullDownload) {
        crcObserver = std::make_shared<CRC64WriteObserver>();
    }
    std::shared_ptr<ProgressWriteObserver> progressObs = nullptr;
    if (request.getProgressCallback().has_value()) {
        progressObs = std::make_shared<ProgressWriteObserver>(request.getProgressCallback().value(), -1);
    }

    std::int64_t offset = 0;
    models::GetObjectRequest req = request;

    for (;;) {
        std::shared_ptr<std::ofstream> fileStream = nullptr;

        SinkFactory factory;
        factory.isOneShot = true;
        factory.supplier = [&fileStream, &filePath, offset, crcObserver, progressObs](
                std::int64_t contentLength) -> std::shared_ptr<ByteWriter> {
            if (offset == 0) {
                fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::trunc);
            } else {
                fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::in | std::ios::out);
                fileStream->seekp(offset);
            }
            auto writer = std::make_shared<OStreamWriter>(fileStream);

            if (!crcObserver && !progressObs) {
                return writer;
            }

            auto observable = std::make_shared<ObservableWriter>(writer);
            if (crcObserver) {
                observable->addObserver(crcObserver);
            }
            if (progressObs) {
                progressObs->updateTotal(contentLength);
                observable->addObserver(progressObs);
            }
            return observable;
        };

        if (offset + rangeStart > 0 || rangeEnd != -1) {
            std::string range = "bytes=" + std::to_string(rangeStart + offset) + "-";
            if (rangeEnd >= 0) {
                range += std::to_string(rangeEnd);
            }
            req.setRange(std::move(range));
            req.setRangeBehavior("standard");
        }
        req.setSinkFactory(factory);

        auto outcome = getObject(req, options);

        if (outcome.has_value()) {
            if (crcObserver && offset == 0) {
                const auto& serverCrc = outcome.value().getHashCrc64ecma();
                if (!serverCrc.empty()) {
                    auto ccrc = crcObserver->crcAsString();
                    if (ccrc != serverCrc) {
                        return makeUnexpected(OperationError(
                                ClientErrorCode::CrcMismatch,
                                {{"Code", "CRCInconsistent"},
                                 {"Message", "crc is inconsistent, client crc:" + ccrc + ", server crc:" + serverCrc}}));
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
