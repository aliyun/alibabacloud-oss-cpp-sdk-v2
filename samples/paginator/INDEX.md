# Paginator Samples

Paginator samples using `makePaginator()`. Automatically handles pagination tokens across pages.

## Usage Pattern

```cpp
#include "alibabacloud/oss2/Paginator.h"

auto paginator = makePaginator(client,
    models::ListObjectsV2Request()
        .setBucket("my-bucket")
        .setMaxKeys(100));

while (paginator.hasNext()) {
    auto outcome = paginator.nextPage();
    if (!outcome.has_value()) break;
    for (const auto& obj : outcome.value().getContents()) {
        // process obj
    }
}
```

## Samples

| Use Case | File | Paginated API |
|----------|------|---------------|
| List all objects (v2) | ListObjectsV2Paginator.cpp | `client.listObjectsV2()` |
| List all objects (v1) | ListObjectsPaginator.cpp | `client.listObjects()` |
| List all buckets | ListBucketsPaginator.cpp | `client.listBuckets()` |
| List all object versions | ListObjectVersionsPaginator.cpp | `client.listObjectVersions()` |
| List all ongoing uploads | ListMultipartUploadsPaginator.cpp | `client.listMultipartUploads()` |
| List all parts of an upload | ListPartsPaginator.cpp | `client.listParts()` |
