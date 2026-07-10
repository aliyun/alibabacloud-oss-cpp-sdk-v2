#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/agentic/models/AgenticBucket.h"
#include "alibabacloud/oss2/utils/Outcome.h"

#include <functional>

namespace alibabacloud {
namespace oss2 {
namespace agentic {

using CreateAgenticBucketOutcome = Outcome<models::CreateAgenticBucketResult, OperationError>;
using DeleteAgenticBucketOutcome = Outcome<models::DeleteAgenticBucketResult, OperationError>;
using GetAgenticBucketOutcome = Outcome<models::GetAgenticBucketResult, OperationError>;
using ListAgenticBucketsOutcome = Outcome<models::ListAgenticBucketsResult, OperationError>;
using PutAgenticBucketStatusOutcome = Outcome<models::PutAgenticBucketStatusResult, OperationError>;
using ListBucketSpacesOutcome = Outcome<models::ListBucketSpacesResult, OperationError>;

using CreateAgenticBucketAsyncCallback = std::function<void(CreateAgenticBucketOutcome)>;
using DeleteAgenticBucketAsyncCallback = std::function<void(DeleteAgenticBucketOutcome)>;
using GetAgenticBucketAsyncCallback = std::function<void(GetAgenticBucketOutcome)>;
using ListAgenticBucketsAsyncCallback = std::function<void(ListAgenticBucketsOutcome)>;
using PutAgenticBucketStatusAsyncCallback = std::function<void(PutAgenticBucketStatusOutcome)>;
using ListBucketSpacesAsyncCallback = std::function<void(ListBucketSpacesOutcome)>;

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
