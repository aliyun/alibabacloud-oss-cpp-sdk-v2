# Sync API Samples

Sync samples using `OSSClient`. Each file is a standalone executable.

## Error Handling

All samples follow this pattern:
```cpp
auto outcome = client.xxxApi(models::XxxRequest()...);
if (!outcome.has_value()) {
    auto& e = outcome.error();
    std::cerr << "Fail, code: " << e.getCode()
              << ", message: " << e.getMessage()
              << ", ec: " << e.getEC()
              << ", requestId: " << e.getRequestId()
              << ", requestTarget: " << e.getRequestTarget() << std::endl;
    return 1;
}
auto& result = outcome.value();
// use result...
```

## Object Operations

| Use Case | File | Key API |
|----------|------|---------|
| Upload a string as an object | PutObject.cpp | `client.putObject()` |
| Download an object | GetObject.cpp | `client.getObject()` |
| Copy an object | CopyObject.cpp | `client.copyObject()` |
| Append data to an object | AppendObject.cpp | `client.appendObject()` |
| Seal an appendable object | SealAppendObject.cpp | `client.sealAppendObject()` |
| Delete a single object | DeleteObject.cpp | `client.deleteObject()` |
| Delete multiple objects | DeleteMultipleObjects.cpp | `client.deleteMultipleObjects()` |
| Query object metadata (HEAD) | HeadObject.cpp | `client.headObject()` |
| Query lightweight metadata | GetObjectMeta.cpp | `client.getObjectMeta()` |
| Restore an Archive object | RestoreObject.cpp | `client.restoreObject()` |
| Clean a restored object | CleanRestoredObject.cpp | `client.cleanRestoredObject()` |

## Object ACL

| Use Case | File | Key API |
|----------|------|---------|
| Set object ACL | PutObjectAcl.cpp | `client.putObjectAcl()` |
| Query object ACL | GetObjectAcl.cpp | `client.getObjectAcl()` |

## Symbolic Links

| Use Case | File | Key API |
|----------|------|---------|
| Create a symlink | PutSymlink.cpp | `client.putSymlink()` |
| Query a symlink target | GetSymlink.cpp | `client.getSymlink()` |

## Object Tagging

| Use Case | File | Key API |
|----------|------|---------|
| Add/update tags | PutObjectTagging.cpp | `client.putObjectTagging()` |
| Query tags | GetObjectTagging.cpp | `client.getObjectTagging()` |
| Delete tags | DeleteObjectTagging.cpp | `client.deleteObjectTagging()` |

## Multipart Upload

| Use Case | File | Key API |
|----------|------|---------|
| Full multipart upload flow | CompleteMultipartUpload.cpp | `initiate` + `uploadPart` + `complete` |
| Initiate upload | InitiateMultipartUpload.cpp | `client.initiateMultipartUpload()` |
| Upload a part | UploadPart.cpp | `client.uploadPart()` |
| Upload part by copy | UploadPartCopy.cpp | `client.uploadPartCopy()` |
| Abort upload | AbortMultipartUpload.cpp | `client.abortMultipartUpload()` |
| List parts | ListParts.cpp | `client.listParts()` |
| List ongoing uploads | ListMultipartUploads.cpp | `client.listMultipartUploads()` |

## Bucket Operations

| Use Case | File | Key API |
|----------|------|---------|
| Create a bucket | PutBucket.cpp | `client.putBucket()` |
| Delete a bucket | DeleteBucket.cpp | `client.deleteBucket()` |
| Query bucket info | GetBucketInfo.cpp | `client.getBucketInfo()` |
| Query bucket region | GetBucketLocation.cpp | `client.getBucketLocation()` |
| Query bucket statistics | GetBucketStat.cpp | `client.getBucketStat()` |
| Set bucket ACL | PutBucketAcl.cpp | `client.putBucketAcl()` |
| Query bucket ACL | GetBucketAcl.cpp | `client.getBucketAcl()` |
| Set Referer whitelist | PutBucketReferer.cpp | `client.putBucketReferer()` |
| Query Referer config | GetBucketReferer.cpp | `client.getBucketReferer()` |
| Enable/suspend versioning | PutBucketVersioning.cpp | `client.putBucketVersioning()` |
| Query versioning state | GetBucketVersioning.cpp | `client.getBucketVersioning()` |
| List object versions | ListObjectVersions.cpp | `client.listObjectVersions()` |

## Service

| Use Case | File | Key API |
|----------|------|---------|
| List all buckets | ListBuckets.cpp | `client.listBuckets()` |
| List objects (v1) | ListObjects.cpp | `client.listObjects()` |
| List objects (v2) | ListObjectsV2.cpp | `client.listObjectsV2()` |
| Query region endpoints | DescribeRegions.cpp | `client.describeRegions()` |

## Presigned URLs

| Use Case | File | Key API |
|----------|------|---------|
| Presign PutObject | PresignPutObject.cpp | `client.presign(PutObjectRequest)` |
| Presign GetObject | PresignGetObject.cpp | `client.presign(GetObjectRequest)` |
| Presign HeadObject | PresignHeadObject.cpp | `client.presign(HeadObjectRequest)` |
| Presign UploadPart | PresignUploadPart.cpp | `client.presign(UploadPartRequest)` |

## Data Processing

| Use Case | File | Key API |
|----------|------|---------|
| Process an object (e.g. image resize) | ProcessObject.cpp | `client.processObject()` |
| Async process (e.g. video transcode) | AsyncProcessObject.cpp | `client.asyncProcessObject()` |

## Extension

| Use Case | File | Key API |
|----------|------|---------|
| Upload a local file | PutObjectFromFile.cpp | `client.putObjectFromFile()` |
| Download to a local file (resumable) | GetObjectToFile.cpp | `client.getObjectToFile()` |
| Check if an object exists | IsObjectExist.cpp | `client.isObjectExist()` |
| Check if a bucket exists | IsBucketExist.cpp | `client.isBucketExist()` |

## Advanced

| Use Case | File | Key API |
|----------|------|---------|
| Low-level API call | InvokeOperation.cpp | `client.invokeOperation()` |
