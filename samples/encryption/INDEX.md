# Encryption Samples

Client-side encryption samples using `OSSEncryptionClient`. Each file is a standalone executable.

Requires `-DENABLE_ENCRYPTION=ON` to build.

| Use Case | File | Key API |
|----------|------|---------|
| Encrypted upload and download | PutGetObject.cpp | `OSSEncryptionClient::putObject()` / `getObject()` |
| Encrypted multipart upload | MultipartUpload.cpp | `initiateMultipartUpload()` + `uploadPart()` + `completeMultipartUpload()` |

## Run

```bash
# PEM key files are required
./sample_encryption_PutGetObject --region cn-hangzhou --bucket my-bucket --key my-key \
    --public-key-file public_key.pem --private-key-file private_key.pem

./sample_encryption_MultipartUpload --region cn-hangzhou --bucket my-bucket --key my-key \
    --public-key-file public_key.pem --private-key-file private_key.pem
```
