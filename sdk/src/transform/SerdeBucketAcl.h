#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/BucketAcl.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutBucketAcl(const models::PutBucketAclRequest& request);
Outcome<models::PutBucketAclResult, OperationError> toPutBucketAcl(OperationOutput&& output);


OperationInput fromGetBucketAcl(const models::GetBucketAclRequest& request);
Outcome<models::GetBucketAclResult, OperationError> toGetBucketAcl(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud