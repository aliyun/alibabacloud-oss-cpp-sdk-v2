
#include "OSSClientUtils.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "src/internal/ByteStreamUtils.h"
#include "src/internal/SelectFrameDecoder.h"
#include "src/internal/sync/ClientImpl.h"
#include "src/transform/SerdeObjectBasic.h"
#include "src/transform/SerdeObjectSelect.h"
#include "src/utils/Utils.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {


// Object Basic
PutObjectOutcome OSSClient::putObject(const models::PutObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromPutObject(request);
    if (client_->hasFlag(FeatureFlagsType::AutoDetectMimeType)) {
        if (input.headers.find("Content-Type") == input.headers.end()) {
            // cppcheck-suppress stlFindInsert
            input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
        }
    }

    internal::OperationInnerOptions innerOpts;
    if (request.getProgressCallback().has_value()) {
        int64_t total = 0;
        if (request.getBody()) {
            auto len = request.getBody()->length();
            total = len.has_value() ? static_cast<int64_t>(len.value()) : -1;
        }
        innerOpts.uploadObserver.push_back(
            std::make_shared<internal::ProgressObserver>(request.getProgressCallback().value(), total));
    }

    if (client_->hasFlag(FeatureFlagsType::EnableCRC64CheckUpload) && request.hasBody()) {
        auto crcObserver = std::make_shared<internal::CRC64Observer>(0);
        innerOpts.uploadObserver.push_back(crcObserver);
        innerOpts.onResponseMessage.emplace_back(internal::CRC64ResponseChecker{crcObserver});
    }

    auto result = client_->Execute(input, options, &innerOpts);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toPutObject(std::move(std::get<OperationOutput>(result)));
}

CopyObjectOutcome OSSClient::copyObject(const models::CopyObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredFieldsOr(SourceKey, CopySource);

    auto input = transform::fromCopyObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toCopyObject(std::move(std::get<OperationOutput>(result)));
}

GetObjectOutcome OSSClient::getObject(const models::GetObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObject(request);

    internal::OperationInnerOptions innerOpts;
    if (request.getSinkFactory().has_value()) {
        innerOpts.sinkFactory = request.getSinkFactory();
    }

    auto result = client_->Execute(input, options, &innerOpts);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetObject(std::move(std::get<OperationOutput>(result)));
}

AppendObjectOutcome OSSClient::appendObject(const models::AppendObjectRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Position);

    auto input = transform::fromAppendObject(request);
    if (client_->hasFlag(FeatureFlagsType::AutoDetectMimeType)) {
        if (input.headers.find("Content-Type") == input.headers.end()) {
            // cppcheck-suppress stlFindInsert
            input.headers.emplace("Content-Type", utils::LookupMimeType(request.getKey()));
        }
    }

    internal::OperationInnerOptions innerOpts;
    std::shared_ptr<internal::CRC64Observer> crcObserver;
    if (client_->hasFlag(FeatureFlagsType::EnableCRC64CheckUpload) && request.hasBody()
        && request.getInitHashCRC64().has_value()) {
        crcObserver = std::make_shared<internal::CRC64Observer>(request.getInitHashCRC64().value());
        innerOpts.uploadObserver.push_back(crcObserver);
    }

    auto result = client_->Execute(input, options, &innerOpts);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }

    auto appendOutcome = transform::toAppendObject(std::move(std::get<OperationOutput>(result)));

    if (crcObserver && appendOutcome.has_value()) {
        const auto& serverCrc = appendOutcome.value().getHashCrc64ecma();
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

    return appendOutcome;
}

SealAppendObjectOutcome OSSClient::sealAppendObject(const models::SealAppendObjectRequest& request,
                                                    const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Position);

    auto input = transform::fromSealAppendObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toSealAppendObject(std::move(std::get<OperationOutput>(result)));
}

DeleteObjectOutcome OSSClient::deleteObject(const models::DeleteObjectRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromDeleteObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toDeleteObject(std::move(std::get<OperationOutput>(result)));
}

DeleteMultipleObjectsOutcome OSSClient::deleteMultipleObjects(const models::DeleteMultipleObjectsRequest& request,
                                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredHasField(Delete);

    auto input = transform::fromDeleteMultipleObjects(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toDeleteMultipleObjects(std::move(std::get<OperationOutput>(result)));
}

HeadObjectOutcome OSSClient::headObject(const models::HeadObjectRequest& request, const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromHeadObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toHeadObject(std::move(std::get<OperationOutput>(result)));
}

GetObjectMetaOutcome OSSClient::getObjectMeta(const models::GetObjectMetaRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromGetObjectMeta(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toGetObjectMeta(std::move(std::get<OperationOutput>(result)));
}

RestoreObjectOutcome OSSClient::restoreObject(const models::RestoreObjectRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromRestoreObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toRestoreObject(std::move(std::get<OperationOutput>(result)));
}

CleanRestoredObjectOutcome OSSClient::cleanRestoredObject(const models::CleanRestoredObjectRequest& request,
                                                          const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromCleanRestoredObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toCleanRestoredObject(std::move(std::get<OperationOutput>(result)));
}

ProcessObjectOutcome OSSClient::processObject(const models::ProcessObjectRequest& request,
                                              const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromProcessObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toProcessObject(std::move(std::get<OperationOutput>(result)));
}

AsyncProcessObjectOutcome OSSClient::asyncProcessObject(const models::AsyncProcessObjectRequest& request,
                                                        const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    auto input = transform::fromAsyncProcessObject(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toAsyncProcessObject(std::move(std::get<OperationOutput>(result)));
}


// Object Select
SelectObjectOutcome OSSClient::selectObject(const models::SelectObjectRequest& request,
                                            const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Process);
    requiredHasField(SelectRequest);

    auto input = transform::fromSelectObject(request);

    internal::OperationInnerOptions innerOpts;

    auto userFactory = request.getSinkFactory();
    std::shared_ptr<std::stringstream> defaultStream;
    if (!userFactory.has_value()) {
        defaultStream = std::make_shared<std::stringstream>();
    }

    SinkFactory factory;
    factory.isOneShot = userFactory.has_value() ? userFactory->isOneShot : false;
    factory.supplier = [userFactory, defaultStream](std::int64_t size,
                                                    const HeaderCollection& headers) -> std::shared_ptr<ByteWriter> {
        std::shared_ptr<ByteWriter> userSink;
        if (userFactory.has_value()) {
            userSink = userFactory.value()(size, headers);
        } else {
            defaultStream->str("");
            defaultStream->clear();
            userSink = std::make_shared<OStreamWriter>(defaultStream);
        }

        auto it = headers.find("x-oss-select-output-raw");
        if (it != headers.end() && it->second == "true") {
            return userSink;
        }
        return std::make_shared<internal::SelectFrameDecodingWriter>(userSink);
    };
    innerOpts.sinkFactory = factory;

    auto result = client_->Execute(input, options, &innerOpts);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    auto outcome = transform::toSelectObject(std::move(std::get<OperationOutput>(result)));
    if (outcome.has_value() && defaultStream) {
        outcome.value().setBody(defaultStream);
    }
    return outcome;
}

CreateSelectObjectMetaOutcome OSSClient::createSelectObjectMeta(const models::CreateSelectObjectMetaRequest& request,
                                                                const OperationOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(Process);
    requiredHasField(SelectMetaRequest);

    auto input = transform::fromCreateSelectObjectMeta(request);
    auto result = client_->Execute(input, options);
    if (std::holds_alternative<OperationError>(result)) {
        return makeUnexpected(std::get<OperationError>(result));
    }
    return transform::toCreateSelectObjectMeta(std::move(std::get<OperationOutput>(result)));
}

} // namespace oss2
} // namespace alibabacloud
