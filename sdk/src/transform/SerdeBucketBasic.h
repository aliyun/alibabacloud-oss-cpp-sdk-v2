#pragma once

#include "alibabacloud/oss2/Operation.h"

#include "alibabacloud/oss2/models/BucketBasic.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromGetBucketStat(const models::GetBucketStatRequest& request);
Outcome<models::GetBucketStatResult, OperationError> toGetBucketStat(OperationOutput&& output);


OperationInput fromPutBucket(const models::PutBucketRequest& request);
Outcome<models::PutBucketResult, OperationError> toPutBucket(OperationOutput&& output);


OperationInput fromDeleteBucket(const models::DeleteBucketRequest& request);
Outcome<models::DeleteBucketResult, OperationError> toDeleteBucket(OperationOutput&& output);


OperationInput fromListObjects(const models::ListObjectsRequest& request);
Outcome<models::ListObjectsResult, OperationError> toListObjects(OperationOutput&& output);


OperationInput fromListObjectsV2(const models::ListObjectsV2Request& request);
Outcome<models::ListObjectsV2Result, OperationError> toListObjectsV2(OperationOutput&& output);


OperationInput fromGetBucketInfo(const models::GetBucketInfoRequest& request);
Outcome<models::GetBucketInfoResult, OperationError> toGetBucketInfo(OperationOutput&& output);


OperationInput fromGetBucketLocation(const models::GetBucketLocationRequest& request);
Outcome<models::GetBucketLocationResult, OperationError> toGetBucketLocation(OperationOutput&& output);


} // namespace transform
} // namespace oss2
} // namespace alibabacloud