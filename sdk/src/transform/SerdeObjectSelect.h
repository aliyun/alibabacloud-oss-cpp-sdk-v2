#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/ObjectSelect.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromSelectObject(const models::SelectObjectRequest& request);
Outcome<models::SelectObjectResult, OperationError> toSelectObject(OperationOutput&& output);


OperationInput fromCreateSelectObjectMeta(const models::CreateSelectObjectMetaRequest& request);
Outcome<models::CreateSelectObjectMetaResult, OperationError> toCreateSelectObjectMeta(OperationOutput&& output);

} // namespace transform
} // namespace oss2
} // namespace alibabacloud
