
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"
#include "src/internal/async/AsyncClientImpl.h"

#include <fstream>
#include <memory>

namespace alibabacloud {
namespace oss2 {

void OSSAsyncClient::putObjectFromFileAsync(const models::PutObjectRequest& request,
                                            const std::string& filePath,
                                            const PutObjectAsyncCallback& callback,
                                            const OperationOptions* options) {
    auto req = request;
    req.setBody(RequestBody::fromFile(filePath));
    putObjectAsync(req, callback, options);
}

void OSSAsyncClient::isObjectExistAsync(const std::string& bucket, const std::string& key,
                                        const BoolAsyncCallback& callback,
                                        const OperationOptions* options) {
    getObjectMetaAsync(
            models::GetObjectMetaRequest().setBucket(bucket).setKey(key),
            [callback](GetObjectMetaOutcome outcome) {
                if (outcome.has_value()) {
                    callback(true);
                    return;
                }
                const auto& e = outcome.error();
                if (e.getCode() == "NoSuchKey" ||
                    (e.getStatusCode() == 404 && e.getCode() == "BadErrorResponse")) {
                    callback(false);
                    return;
                }
                callback(makeUnexpected(std::move(outcome.error())));
            }, options);
}

void OSSAsyncClient::isBucketExistAsync(const std::string& bucket,
                                        const BoolAsyncCallback& callback,
                                        const OperationOptions* options) {
    getBucketAclAsync(
            models::GetBucketAclRequest().setBucket(bucket),
            [callback](GetBucketAclOutcome outcome) {
                if (outcome.has_value()) {
                    callback(true);
                    return;
                }
                const auto& e = outcome.error();
                if (e.getCode() == "NoSuchBucket") {
                    callback(false);
                    return;
                }
                if (e.getStatusCode() > 0) {
                    callback(true);
                    return;
                }
                callback(makeUnexpected(std::move(outcome.error())));
            }, options);
}

void OSSAsyncClient::getObjectToFileAsync(const models::GetObjectRequest& request,
                                          const std::string& filePath,
                                          const GetObjectAsyncCallback& callback,
                                          const OperationOptions* options) {
    std::shared_ptr<CRC64WriteObserver> crcObserver = nullptr;
    if (client_->hasFlag(FeatureFlagsType::EnableCRC64CheckDownload) && request.getRange().empty()) {
        crcObserver = std::make_shared<CRC64WriteObserver>();
    }
    std::shared_ptr<ProgressWriteObserver> progressObs = nullptr;
    if (request.getProgressCallback().has_value()) {
        progressObs = std::make_shared<ProgressWriteObserver>(request.getProgressCallback().value(), -1);
    }

    SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [filePath, crcObserver, progressObs](
            std::int64_t contentLength, const HeaderCollection&) -> std::shared_ptr<ByteWriter> {
        auto fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::trunc);
        auto writer = std::make_shared<OStreamWriter>(fileStream);

        if (!crcObserver && !progressObs) {
            return writer;
        }

        auto observable = std::make_shared<ObservableWriter>(writer);
        if (crcObserver) {
            crcObserver->reset();
            observable->addObserver(crcObserver);
        }
        if (progressObs) {
            progressObs->updateTotal(contentLength);
            observable->addObserver(progressObs);
        }
        return observable;
    };

    auto req = request;
    req.setSinkFactory(factory);

    getObjectAsync(req, [callback, crcObserver](GetObjectOutcome outcome) {
        if (outcome.has_value() && crcObserver) {
            const auto& serverCrc = outcome.value().getHashCrc64ecma();
            if (!serverCrc.empty()) {
                auto ccrc = crcObserver->crcAsString();
                if (ccrc != serverCrc) {
                    callback(makeUnexpected(OperationError(
                            ClientErrorCode::CrcMismatch,
                            {{"Code", "CRCInconsistent"},
                             {"Message", "crc is inconsistent, client crc:" + ccrc + ", server crc:" + serverCrc}})));
                    return;
                }
            }
        }
        callback(std::move(outcome));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
