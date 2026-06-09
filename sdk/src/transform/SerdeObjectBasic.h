#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectBasic.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutObject(const models::PutObjectRequest& request);
Outcome<models::PutObjectResult, OperationError> toPutObject(OperationOutput&& output);


OperationInput fromCopyObject(const models::CopyObjectRequest& request);
Outcome<models::CopyObjectResult, OperationError> toCopyObject(OperationOutput&& output);


OperationInput fromGetObject(const models::GetObjectRequest& request);
Outcome<models::GetObjectResult, OperationError> toGetObject(OperationOutput&& output);


OperationInput fromAppendObject(const models::AppendObjectRequest& request);
Outcome<models::AppendObjectResult, OperationError> toAppendObject(OperationOutput&& output);


OperationInput fromSealAppendObject(const models::SealAppendObjectRequest& request);
Outcome<models::SealAppendObjectResult, OperationError> toSealAppendObject(OperationOutput&& output);


OperationInput fromDeleteObject(const models::DeleteObjectRequest& request);
Outcome<models::DeleteObjectResult, OperationError> toDeleteObject(OperationOutput&& output);


OperationInput fromDeleteMultipleObjects(const models::DeleteMultipleObjectsRequest& request);
Outcome<models::DeleteMultipleObjectsResult, OperationError> toDeleteMultipleObjects(OperationOutput&& output);


OperationInput fromHeadObject(const models::HeadObjectRequest& request);
Outcome<models::HeadObjectResult, OperationError> toHeadObject(OperationOutput&& output);


OperationInput fromGetObjectMeta(const models::GetObjectMetaRequest& request);
Outcome<models::GetObjectMetaResult, OperationError> toGetObjectMeta(OperationOutput&& output);


OperationInput fromRestoreObject(const models::RestoreObjectRequest& request);
Outcome<models::RestoreObjectResult, OperationError> toRestoreObject(OperationOutput&& output);


OperationInput fromCleanRestoredObject(const models::CleanRestoredObjectRequest& request);
Outcome<models::CleanRestoredObjectResult, OperationError> toCleanRestoredObject(OperationOutput&& output);


OperationInput fromProcessObject(const models::ProcessObjectRequest& request);
Outcome<models::ProcessObjectResult, OperationError> toProcessObject(OperationOutput&& output);


OperationInput fromAsyncProcessObject(const models::AsyncProcessObjectRequest& request);
Outcome<models::AsyncProcessObjectResult, OperationError> toAsyncProcessObject(OperationOutput&& output);

} // namespace transform
} // namespace oss2
} // namespace alibabacloud