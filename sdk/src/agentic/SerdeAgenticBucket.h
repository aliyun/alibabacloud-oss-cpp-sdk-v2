#pragma once
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/agentic/models/AgenticBucket.h"

namespace alibabacloud {
namespace oss2 {
namespace agentic {
namespace transform {

OperationInput fromCreateAgenticBucket(const models::CreateAgenticBucketRequest& request);
Outcome<models::CreateAgenticBucketResult, OperationError> toCreateAgenticBucket(OperationOutput&& output);

OperationInput fromDeleteAgenticBucket(const models::DeleteAgenticBucketRequest& request);
Outcome<models::DeleteAgenticBucketResult, OperationError> toDeleteAgenticBucket(OperationOutput&& output);

OperationInput fromGetAgenticBucket(const models::GetAgenticBucketRequest& request);
Outcome<models::GetAgenticBucketResult, OperationError> toGetAgenticBucket(OperationOutput&& output);

OperationInput fromListAgenticBuckets(const models::ListAgenticBucketsRequest& request);
Outcome<models::ListAgenticBucketsResult, OperationError> toListAgenticBuckets(OperationOutput&& output);

OperationInput fromPutAgenticBucketStatus(const models::PutAgenticBucketStatusRequest& request);
Outcome<models::PutAgenticBucketStatusResult, OperationError> toPutAgenticBucketStatus(OperationOutput&& output);

OperationInput fromListBucketSpaces(const models::ListBucketSpacesRequest& request);
Outcome<models::ListBucketSpacesResult, OperationError> toListBucketSpaces(OperationOutput&& output);

} // namespace transform
} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
