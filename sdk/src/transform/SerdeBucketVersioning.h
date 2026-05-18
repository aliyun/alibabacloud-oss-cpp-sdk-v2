#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/BucketVersioning.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutBucketVersioning(const models::PutBucketVersioningRequest& request);
Outcome<models::PutBucketVersioningResult, OperationError> toPutBucketVersioning(OperationOutput&& output);


OperationInput fromGetBucketVersioning(const models::GetBucketVersioningRequest& request);
Outcome<models::GetBucketVersioningResult, OperationError> toGetBucketVersioning(OperationOutput&& output);


OperationInput fromListObjectVersions(const models::ListObjectVersionsRequest& request);
Outcome<models::ListObjectVersionsResult, OperationError> toListObjectVersions(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud
