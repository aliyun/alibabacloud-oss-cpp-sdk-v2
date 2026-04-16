#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/utils/Outcome.h"

#include "alibabacloud/oss2/models/BucketAcl.h"
#include "alibabacloud/oss2/models/BucketBasic.h"
#include "alibabacloud/oss2/models/BucketReferer.h"
#include "alibabacloud/oss2/models/ObjectAcl.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/models/ObjectMultipart.h"
#include "alibabacloud/oss2/models/ObjectSymlink.h"
#include "alibabacloud/oss2/models/ObjectTagging.h"
#include "alibabacloud/oss2/models/Presign.h"
#include "alibabacloud/oss2/models/Region.h"
#include "alibabacloud/oss2/models/Service.h"


namespace alibabacloud {
namespace oss2 {
using ListBucketsOutcome = Outcome<models::ListBucketsResult, OperationError>;

using DescribeRegionsOutcome = Outcome<models::DescribeRegionsResult, OperationError>;

using GetBucketStatOutcome = Outcome<models::GetBucketStatResult, OperationError>;

using PutBucketOutcome = Outcome<models::PutBucketResult, OperationError>;

using DeleteBucketOutcome = Outcome<models::DeleteBucketResult, OperationError>;

using ListObjectsOutcome = Outcome<models::ListObjectsResult, OperationError>;

using ListObjectsV2Outcome = Outcome<models::ListObjectsV2Result, OperationError>;

using GetBucketInfoOutcome = Outcome<models::GetBucketInfoResult, OperationError>;

using GetBucketLocationOutcome = Outcome<models::GetBucketLocationResult, OperationError>;

using PutBucketAclOutcome = Outcome<models::PutBucketAclResult, OperationError>;

using GetBucketAclOutcome = Outcome<models::GetBucketAclResult, OperationError>;

using PutBucketRefererOutcome = Outcome<models::PutBucketRefererResult, OperationError>;

using GetBucketRefererOutcome = Outcome<models::GetBucketRefererResult, OperationError>;

using PutObjectOutcome = Outcome<models::PutObjectResult, OperationError>;

using CopyObjectOutcome = Outcome<models::CopyObjectResult, OperationError>;

using GetObjectOutcome = Outcome<models::GetObjectResult, OperationError>;

using AppendObjectOutcome = Outcome<models::AppendObjectResult, OperationError>;

using SealAppendObjectOutcome = Outcome<models::SealAppendObjectResult, OperationError>;

using DeleteObjectOutcome = Outcome<models::DeleteObjectResult, OperationError>;

using DeleteMultipleObjectsOutcome = Outcome<models::DeleteMultipleObjectsResult, OperationError>;

using HeadObjectOutcome = Outcome<models::HeadObjectResult, OperationError>;

using GetObjectMetaOutcome = Outcome<models::GetObjectMetaResult, OperationError>;

using RestoreObjectOutcome = Outcome<models::RestoreObjectResult, OperationError>;

using CleanRestoredObjectOutcome = Outcome<models::CleanRestoredObjectResult, OperationError>;

using PutObjectAclOutcome = Outcome<models::PutObjectAclResult, OperationError>;

using GetObjectAclOutcome = Outcome<models::GetObjectAclResult, OperationError>;

using PutSymlinkOutcome = Outcome<models::PutSymlinkResult, OperationError>;

using GetSymlinkOutcome = Outcome<models::GetSymlinkResult, OperationError>;

using PutObjectTaggingOutcome = Outcome<models::PutObjectTaggingResult, OperationError>;

using GetObjectTaggingOutcome = Outcome<models::GetObjectTaggingResult, OperationError>;

using DeleteObjectTaggingOutcome = Outcome<models::DeleteObjectTaggingResult, OperationError>;

using InitiateMultipartUploadOutcome = Outcome<models::InitiateMultipartUploadResult, OperationError>;

using UploadPartOutcome = Outcome<models::UploadPartResult, OperationError>;

using CompleteMultipartUploadOutcome = Outcome<models::CompleteMultipartUploadResult, OperationError>;

using UploadPartCopyOutcome = Outcome<models::UploadPartCopyResult, OperationError>;

using AbortMultipartUploadOutcome = Outcome<models::AbortMultipartUploadResult, OperationError>;

using ListMultipartUploadsOutcome = Outcome<models::ListMultipartUploadsResult, OperationError>;

using ListPartsOutcome = Outcome<models::ListPartsResult, OperationError>;

using PresignOutcome = Outcome<models::PresignResult, OperationError>;

} // namespace oss2
} // namespace alibabacloud