
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
    req.setBody(RequestBody::FromFile(filePath));
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
    const bool crcEnabled = client_->hasFlag(FeatureFlagsType::EnableCRC64CheckDownload) &&
                            request.getRange().empty();
    auto crcObserver = crcEnabled ? std::make_shared<CRC64WriteObserver>() : nullptr;
    auto fileStream = std::make_shared<std::shared_ptr<std::ofstream>>();

    SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [fileStream, filePath, crcObserver](
            std::int64_t /*contentLength*/) -> std::shared_ptr<ByteWriter> {
        *fileStream = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ios::trunc);
        auto writer = std::make_shared<OStreamWriter>(*fileStream);
        if (crcObserver) {
            crcObserver->reset();
            return std::make_shared<ObservableWriter>(writer, crcObserver);
        }
        return writer;
    };

    auto req = request;
    req.setSinkFactory(factory);

    getObjectAsync(req, [callback, crcObserver](GetObjectOutcome outcome) {
        if (outcome.has_value() && crcObserver) {
            const auto& serverCrc = outcome.value().getHashCrc64ecma();
            if (!serverCrc.empty()) {
                uint64_t expected = outcome.value().getHashCrc64ecmaAsUint64();
                if (expected != crcObserver->crc()) {
                    callback(makeUnexpected(OperationError(
                            ClientErrorCode::CrcMismatch,
                            {{"Code", "CrcMismatch"},
                             {"Message", "CRC-64 verification failed"}})));
                    return;
                }
            }
        }
        callback(std::move(outcome));
    }, options);
}

} // namespace oss2
} // namespace alibabacloud
