#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/Region.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromDescribeRegions(const models::DescribeRegionsRequest& request);
Outcome<models::DescribeRegionsResult, OperationError> toDescribeRegions(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud