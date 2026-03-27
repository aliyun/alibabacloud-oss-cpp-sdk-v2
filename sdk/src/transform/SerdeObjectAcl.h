#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectAcl.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutObjectAcl(const models::PutObjectAclRequest& request);
Outcome<models::PutObjectAclResult, OperationError> toPutObjectAcl(OperationOutput&& output);


OperationInput fromGetObjectAcl(const models::GetObjectAclRequest& request);
Outcome<models::GetObjectAclResult, OperationError> toGetObjectAcl(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud