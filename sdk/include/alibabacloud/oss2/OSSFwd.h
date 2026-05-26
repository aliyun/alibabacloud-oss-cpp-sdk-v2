#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/utils/Outcome.h"

#include "alibabacloud/oss2/models/BucketAcl.h"
#include "alibabacloud/oss2/models/BucketBasic.h"
#include "alibabacloud/oss2/models/BucketReferer.h"
#include "alibabacloud/oss2/models/BucketVersioning.h"
#include "alibabacloud/oss2/models/ObjectAcl.h"
#include "alibabacloud/oss2/models/ObjectBasic.h"
#include "alibabacloud/oss2/models/ObjectMultipart.h"
#include "alibabacloud/oss2/models/ObjectSymlink.h"
#include "alibabacloud/oss2/models/ObjectTagging.h"
#include "alibabacloud/oss2/models/ObjectSelect.h"
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

using PutBucketVersioningOutcome = Outcome<models::PutBucketVersioningResult, OperationError>;

using GetBucketVersioningOutcome = Outcome<models::GetBucketVersioningResult, OperationError>;

using ListObjectVersionsOutcome = Outcome<models::ListObjectVersionsResult, OperationError>;

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

using SelectObjectOutcome = Outcome<models::SelectObjectResult, OperationError>;

using CreateSelectObjectMetaOutcome = Outcome<models::CreateSelectObjectMetaResult, OperationError>;

using BoolOutcome = Outcome<bool, OperationError>;

// Async Callbacks
using ListBucketsAsyncCallback = std::function<void(ListBucketsOutcome)>;
using DescribeRegionsAsyncCallback = std::function<void(DescribeRegionsOutcome)>;
using GetBucketStatAsyncCallback = std::function<void(GetBucketStatOutcome)>;
using PutBucketAsyncCallback = std::function<void(PutBucketOutcome)>;
using DeleteBucketAsyncCallback = std::function<void(DeleteBucketOutcome)>;
using ListObjectsAsyncCallback = std::function<void(ListObjectsOutcome)>;
using ListObjectsV2AsyncCallback = std::function<void(ListObjectsV2Outcome)>;
using GetBucketInfoAsyncCallback = std::function<void(GetBucketInfoOutcome)>;
using GetBucketLocationAsyncCallback = std::function<void(GetBucketLocationOutcome)>;
using PutBucketAclAsyncCallback = std::function<void(PutBucketAclOutcome)>;
using GetBucketAclAsyncCallback = std::function<void(GetBucketAclOutcome)>;
using PutBucketRefererAsyncCallback = std::function<void(PutBucketRefererOutcome)>;
using GetBucketRefererAsyncCallback = std::function<void(GetBucketRefererOutcome)>;
using PutBucketVersioningAsyncCallback = std::function<void(PutBucketVersioningOutcome)>;
using GetBucketVersioningAsyncCallback = std::function<void(GetBucketVersioningOutcome)>;
using ListObjectVersionsAsyncCallback = std::function<void(ListObjectVersionsOutcome)>;
using PutObjectAsyncCallback = std::function<void(PutObjectOutcome)>;
using CopyObjectAsyncCallback = std::function<void(CopyObjectOutcome)>;
using GetObjectAsyncCallback = std::function<void(GetObjectOutcome)>;
using AppendObjectAsyncCallback = std::function<void(AppendObjectOutcome)>;
using SealAppendObjectAsyncCallback = std::function<void(SealAppendObjectOutcome)>;
using DeleteObjectAsyncCallback = std::function<void(DeleteObjectOutcome)>;
using DeleteMultipleObjectsAsyncCallback = std::function<void(DeleteMultipleObjectsOutcome)>;
using HeadObjectAsyncCallback = std::function<void(HeadObjectOutcome)>;
using GetObjectMetaAsyncCallback = std::function<void(GetObjectMetaOutcome)>;
using RestoreObjectAsyncCallback = std::function<void(RestoreObjectOutcome)>;
using CleanRestoredObjectAsyncCallback = std::function<void(CleanRestoredObjectOutcome)>;
using PutObjectAclAsyncCallback = std::function<void(PutObjectAclOutcome)>;
using GetObjectAclAsyncCallback = std::function<void(GetObjectAclOutcome)>;
using PutSymlinkAsyncCallback = std::function<void(PutSymlinkOutcome)>;
using GetSymlinkAsyncCallback = std::function<void(GetSymlinkOutcome)>;
using PutObjectTaggingAsyncCallback = std::function<void(PutObjectTaggingOutcome)>;
using GetObjectTaggingAsyncCallback = std::function<void(GetObjectTaggingOutcome)>;
using DeleteObjectTaggingAsyncCallback = std::function<void(DeleteObjectTaggingOutcome)>;
using InitiateMultipartUploadAsyncCallback = std::function<void(InitiateMultipartUploadOutcome)>;
using UploadPartAsyncCallback = std::function<void(UploadPartOutcome)>;
using CompleteMultipartUploadAsyncCallback = std::function<void(CompleteMultipartUploadOutcome)>;
using UploadPartCopyAsyncCallback = std::function<void(UploadPartCopyOutcome)>;
using AbortMultipartUploadAsyncCallback = std::function<void(AbortMultipartUploadOutcome)>;
using ListMultipartUploadsAsyncCallback = std::function<void(ListMultipartUploadsOutcome)>;
using ListPartsAsyncCallback = std::function<void(ListPartsOutcome)>;
using SelectObjectAsyncCallback = std::function<void(SelectObjectOutcome)>;
using CreateSelectObjectMetaAsyncCallback = std::function<void(CreateSelectObjectMetaOutcome)>;
using BoolAsyncCallback = std::function<void(BoolOutcome)>;

} // namespace oss2
} // namespace alibabacloud