#include "OSSClientFieldCheck.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "src/internal/sync/ClientImpl.h"

#include <ctime>

namespace alibabacloud {
namespace oss2 {

using internal::PresignInnerOutput;
using internal::PresignInnerResult;

#define requiredField(field)                                                                              \
    do {                                                                                                  \
        if (isFieldMissing(request.get##field())) {                                                       \
            return makeUnexpected(                                                                        \
                OperationError(ClientErrorCode::ArgumentRequired,                                         \
                               {{"Code", "ArgumentRequired"}, {"Message", "Missing field " #field ""}})); \
        }                                                                                                 \
    } while (false)


static void applyPresignOptions(OperationInput& opInput, const models::PresignOptions* options) {
    if (options != nullptr && options->hasExpiration()) {
        opInput.opMetadata.emplace("EXPIRATION_TIME", static_cast<std::int64_t>(options->getExpiration()));
    }
}

static PresignOutcome doPresign(internal::ClientImpl* impl, const OperationInput& opInput) {
    auto result = impl->Presign(opInput, nullptr);

    if (auto* error = std::get_if<OperationError>(&result)) {
        return makeUnexpected(std::move(*error));
    }

    if (auto* output = std::get_if<PresignInnerOutput>(&result)) {
        models::PresignResult presignResult;
        presignResult.setUrl(std::move(output->url));
        presignResult.setMethod(std::move(output->method));
        presignResult.setExpiration(output->expiration);
        presignResult.setSignedHeaders(std::move(output->signedHeaders));
        return PresignOutcome(std::move(presignResult));
    }

    return makeUnexpected(
        OperationError{SignerErrorCode::SignFailed, {{"Code", "Unknown"}, {"Message", "Unknown error"}}});
}

PresignOutcome OSSClient::presign(const models::PutObjectRequest& request, const models::PresignOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    OperationInput opInput{"PutObject", "PUT"};

    // headers
    for (auto& [k, v] : request.getHeaders()) {
        opInput.headers.insert_or_assign(k, v);
    }

    for (auto& [k, v] : request.getMetadata()) {
        opInput.headers.insert_or_assign("x-oss-meta-" + k, v);
    }

    // parameters
    for (auto& [k, v] : request.getParameters()) {
        opInput.parameters.insert_or_assign(k, v);
    }

    // body
    opInput.body = request.getBody();
    opInput.bucket = request.getBucket();
    opInput.key = request.getKey();

    applyPresignOptions(opInput, options);
    return doPresign(client_.get(), opInput);
}

PresignOutcome OSSClient::presign(const models::GetObjectRequest& request, const models::PresignOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    OperationInput opInput{"GetObject", "GET"};

    // headers
    for (auto& [k, v] : request.getHeaders()) {
        opInput.headers.insert_or_assign(k, v);
    }

    // parameters
    for (auto& [k, v] : request.getParameters()) {
        opInput.parameters.insert_or_assign(k, v);
    }

    opInput.bucket = request.getBucket();
    opInput.key = request.getKey();

    applyPresignOptions(opInput, options);
    return doPresign(client_.get(), opInput);
}

PresignOutcome OSSClient::presign(const models::HeadObjectRequest& request, const models::PresignOptions* options) {
    requiredField(Bucket);
    requiredField(Key);

    OperationInput opInput{"HeadObject", "HEAD"};

    // headers
    for (auto& [k, v] : request.getHeaders()) {
        opInput.headers.insert_or_assign(k, v);
    }

    // parameters
    for (auto& [k, v] : request.getParameters()) {
        opInput.parameters.insert_or_assign(k, v);
    }

    opInput.bucket = request.getBucket();
    opInput.key = request.getKey();

    applyPresignOptions(opInput, options);
    return doPresign(client_.get(), opInput);
}

PresignOutcome OSSClient::presign(const models::UploadPartRequest& request, const models::PresignOptions* options) {
    requiredField(Bucket);
    requiredField(Key);
    requiredField(UploadId);
    requiredField(PartNumber);

    OperationInput opInput{"UploadPart", "PUT"};

    // headers
    for (auto& [k, v] : request.getHeaders()) {
        opInput.headers.insert_or_assign(k, v);
    }

    // parameters
    for (auto& [k, v] : request.getParameters()) {
        opInput.parameters.insert_or_assign(k, v);
    }

    // body
    opInput.body = request.getBody();
    opInput.bucket = request.getBucket();
    opInput.key = request.getKey();

    applyPresignOptions(opInput, options);
    return doPresign(client_.get(), opInput);
}

} // namespace oss2
} // namespace alibabacloud
