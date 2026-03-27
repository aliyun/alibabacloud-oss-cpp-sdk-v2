#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/Service.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromListBuckets(const models::ListBucketsRequest& request);
Outcome<models::ListBucketsResult, OperationError> toListBuckets(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud