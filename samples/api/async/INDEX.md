# Async API Samples

Async samples using `OSSAsyncClient`. Two patterns are available:

## Callback Pattern
```cpp
client->putObjectAsync(request, PutObjectAsyncCallback([](PutObjectOutcome outcome) {
    if (!outcome.has_value()) { /* handle error */ }
    auto& result = outcome.value();
    // use result...
}));
// must wait for callback to fire before exiting
```

## Future Pattern (asyncCall)
```cpp
auto future = client->asyncCall(models::GetObjectRequest()...);
auto outcome = future.get();  // blocks until done
if (!outcome.has_value()) { /* handle error */ }
auto& result = outcome.value();
// use result...
```

## Object Operations

| Use Case | File | Patterns |
|----------|------|----------|
| Upload an object | PutObject.cpp | Callback + asyncCall |
| Download an object | GetObject.cpp | Callback + asyncCall |
| Copy an object | CopyObject.cpp | Callback + asyncCall |
| Append data to an object | AppendObject.cpp | Callback + asyncCall |
| Delete a single object | DeleteObject.cpp | Callback + asyncCall |
| Delete multiple objects | DeleteMultipleObjects.cpp | asyncCall |
| Query object metadata (HEAD) | HeadObject.cpp | Callback + asyncCall |
| Query lightweight metadata | GetObjectMeta.cpp | asyncCall |
| Seal an appendable object | SealAppendObject.cpp | asyncCall |
| Restore an Archive object | RestoreObject.cpp | asyncCall |
| Clean a restored object | CleanRestoredObject.cpp | asyncCall |

## Object ACL

| Use Case | File | Patterns |
|----------|------|----------|
| Set object ACL | PutObjectAcl.cpp | asyncCall |
| Query object ACL | GetObjectAcl.cpp | asyncCall |

## Symbolic Links

| Use Case | File | Patterns |
|----------|------|----------|
| Create a symlink | PutSymlink.cpp | asyncCall |
| Query a symlink target | GetSymlink.cpp | asyncCall |

## Object Tagging

| Use Case | File | Patterns |
|----------|------|----------|
| Add/update tags | PutObjectTagging.cpp | asyncCall |
| Query tags | GetObjectTagging.cpp | asyncCall |
| Delete tags | DeleteObjectTagging.cpp | asyncCall |

## Multipart Upload

| Use Case | File | Patterns |
|----------|------|----------|
| Full multipart upload flow | CompleteMultipartUpload.cpp | asyncCall |
| Initiate upload | InitiateMultipartUpload.cpp | asyncCall |
| Upload a part | UploadPart.cpp | asyncCall |
| Upload part by copy | UploadPartCopy.cpp | asyncCall |
| Abort upload | AbortMultipartUpload.cpp | asyncCall |
| List parts | ListParts.cpp | asyncCall |
| List ongoing uploads | ListMultipartUploads.cpp | asyncCall |

## Bucket Operations

| Use Case | File | Patterns |
|----------|------|----------|
| Create a bucket | PutBucket.cpp | asyncCall |
| Delete a bucket | DeleteBucket.cpp | asyncCall |
| Query bucket info | GetBucketInfo.cpp | asyncCall |
| Query bucket region | GetBucketLocation.cpp | asyncCall |
| Query bucket statistics | GetBucketStat.cpp | asyncCall |
| Set bucket ACL | PutBucketAcl.cpp | asyncCall |
| Query bucket ACL | GetBucketAcl.cpp | asyncCall |
| Set Referer whitelist | PutBucketReferer.cpp | asyncCall |
| Query Referer config | GetBucketReferer.cpp | asyncCall |
| Enable/suspend versioning | PutBucketVersioning.cpp | asyncCall |
| Query versioning state | GetBucketVersioning.cpp | asyncCall |
| List object versions | ListObjectVersions.cpp | asyncCall |

## Extension

| Use Case | File | Patterns |
|----------|------|----------|
| Upload a local file | PutObjectFromFile.cpp | Callback |
| Download to a local file | GetObjectToFile.cpp | Callback |
| Check if an object exists | IsObjectExist.cpp | Callback |
| Check if a bucket exists | IsBucketExist.cpp | Callback |

## Service

| Use Case | File | Patterns |
|----------|------|----------|
| List all buckets | ListBuckets.cpp | asyncCall |
| List objects (v1) | ListObjects.cpp | asyncCall |
| List objects (v2) | ListObjectsV2.cpp | Callback + asyncCall |
| Query region endpoints | DescribeRegions.cpp | asyncCall |
