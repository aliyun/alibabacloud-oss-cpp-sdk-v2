#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/BucketReferer.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutBucketReferer(const models::PutBucketRefererRequest& request);
Outcome<models::PutBucketRefererResult, OperationError> toPutBucketReferer(OperationOutput&& output);


OperationInput fromGetBucketReferer(const models::GetBucketRefererRequest& request);
Outcome<models::GetBucketRefererResult, OperationError> toGetBucketReferer(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud