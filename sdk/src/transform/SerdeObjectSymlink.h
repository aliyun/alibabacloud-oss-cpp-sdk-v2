#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectSymlink.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutSymlink(const models::PutSymlinkRequest& request);
Outcome<models::PutSymlinkResult, OperationError> toPutSymlink(OperationOutput&& output);


OperationInput fromGetSymlink(const models::GetSymlinkRequest& request);
Outcome<models::GetSymlinkResult, OperationError> toGetSymlink(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud