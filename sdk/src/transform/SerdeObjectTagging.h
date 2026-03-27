#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectTagging.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutObjectTagging(const models::PutObjectTaggingRequest& request);
Outcome<models::PutObjectTaggingResult, OperationError> toPutObjectTagging(OperationOutput&& output);


OperationInput fromGetObjectTagging(const models::GetObjectTaggingRequest& request);
Outcome<models::GetObjectTaggingResult, OperationError> toGetObjectTagging(OperationOutput&& output);


OperationInput fromDeleteObjectTagging(const models::DeleteObjectTaggingRequest& request);
Outcome<models::DeleteObjectTaggingResult, OperationError> toDeleteObjectTagging(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud