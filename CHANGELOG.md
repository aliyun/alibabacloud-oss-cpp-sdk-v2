# ChangeLog - Alibaba Cloud OSS SDK for C++ v2

## Version: 0.2.0 Date: 2026-09-03
### Changes
- Feature: Add agentic bucket


## Version: 0.1.2 Date: 2026-08-21
### Changes
- Fix: Signers overwrite the Authorization header when re-signing a request on retry


## Version: 0.1.1 Date: 2026-06-26
### Changes
- Feature: Add ProcessObject and AsyncProcessObject APIs
- Feature: Add HTTP transport metrics collection
- Feature: Add USE_SYSTEM_TINYXML2 option to support external tinyxml2
- BreakChange: Rename SelectObject model fields to align with OSS XML element names


## Version: 0.1.0 (beta) Date: 2026-06-01
### Changes
- Feature：Add credentials provider
- Feature：Add retryer
- Feature：Add signer v1 and signer v4
- Feature：Add http client based on libcurl and WinHTTP
- Feature：Add bucket's basic api
- Feature：Add object's basic api
- Feature：Add multipart upload api
- Feature：Add select object api
- Feature：Add describe regions api
- Feature：Add presigner
- Feature：Add paginator
- Feature：Add PutObjectFromFile/GetObjectToFile api
- Feature：Add IsObjectExist/IsBucketExist api
- Feature：Add CRC64 integrity check
- Feature：Add additional headers option
- Feature：Add user agent option
- Feature：Add encryption client
- Feature：Add sync and async client
