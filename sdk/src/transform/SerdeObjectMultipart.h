#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectMultipart.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromInitiateMultipartUpload(const models::InitiateMultipartUploadRequest& request);
Outcome<models::InitiateMultipartUploadResult, OperationError> toInitiateMultipartUpload(OperationOutput&& output);


OperationInput fromUploadPart(const models::UploadPartRequest& request);
Outcome<models::UploadPartResult, OperationError> toUploadPart(OperationOutput&& output);


OperationInput fromCompleteMultipartUpload(const models::CompleteMultipartUploadRequest& request);
Outcome<models::CompleteMultipartUploadResult, OperationError> toCompleteMultipartUpload(OperationOutput&& output, bool hasCallback = false);


OperationInput fromUploadPartCopy(const models::UploadPartCopyRequest& request);
Outcome<models::UploadPartCopyResult, OperationError> toUploadPartCopy(OperationOutput&& output);


OperationInput fromAbortMultipartUpload(const models::AbortMultipartUploadRequest& request);
Outcome<models::AbortMultipartUploadResult, OperationError> toAbortMultipartUpload(OperationOutput&& output);


OperationInput fromListMultipartUploads(const models::ListMultipartUploadsRequest& request);
Outcome<models::ListMultipartUploadsResult, OperationError> toListMultipartUploads(OperationOutput&& output);


OperationInput fromListParts(const models::ListPartsRequest& request);
Outcome<models::ListPartsResult, OperationError> toListParts(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud