// Demonstrates: Encrypted multipart upload with OSSEncryptionClient.
//
// Usage:
//   ./sample_encryption_MultipartUpload --region <region> --bucket <bucket> --key <key> \
//       --public-key-file <path> --private-key-file <path>
#include "SampleConfig.h"

#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/RsaMasterCipher.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace oss = alibabacloud::oss2;

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Cannot open file: " << path << std::endl;
        exit(1);
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, key, pubKeyFile, privKeyFile;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--key" && i + 1 < argc) key = argv[++i];
        else if (a == "--public-key-file" && i + 1 < argc) pubKeyFile = argv[++i];
        else if (a == "--private-key-file" && i + 1 < argc) privKeyFile = argv[++i];
    }
    if (region.empty() || bucket.empty() || key.empty() ||
        pubKeyFile.empty() || privKeyFile.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket> --key <key>"
                  << " --public-key-file <path> --private-key-file <path>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto masterCipher = oss::crypto::makeRsaMasterCipher(
        readFile(pubKeyFile), readFile(privKeyFile));

    oss::crypto::EncryptionConfiguration encConfig;
    encConfig.masterCipher = masterCipher;
    oss::OSSEncryptionClient client(conf, std::move(encConfig));

    // Generate test data: 3 parts of 102400 bytes + a partial last part
    int64_t partSize = 102400;
    std::string content(partSize * 3 + 500, 'A');
    int64_t dataSize = static_cast<int64_t>(content.size());

    // 1. Initiate multipart upload
    auto initOutcome = client.initiateMultipartUpload(
        oss::models::InitiateMultipartUploadRequest()
            .setBucket(bucket)
            .setKey(key)
            .setCsePartSize(partSize)
            .setCseDataSize(dataSize));
    if (!initOutcome.has_value()) {
        auto& e = initOutcome.error();
        std::cerr << "InitiateMultipartUpload fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    auto uploadId = initOutcome->getUploadId();
    auto ctx = initOutcome->getCseMultiPartContext();
    std::cout << "Initiated, uploadId: " << uploadId << std::endl;

    // 2. Upload parts
    int partCount = static_cast<int>((dataSize + partSize - 1) / partSize);
    std::vector<oss::models::Part> parts;

    for (int i = 0; i < partCount; i++) {
        int64_t offset = partSize * i;
        int64_t thisPartSize = std::min(partSize, dataSize - offset);
        std::string partData = content.substr(
            static_cast<size_t>(offset), static_cast<size_t>(thisPartSize));

        auto partOutcome = client.uploadPart(
            oss::models::UploadPartRequest()
                .setBucket(bucket)
                .setKey(key)
                .setUploadId(uploadId)
                .setPartNumber(i + 1)
                .setBody(oss::RequestBody::FromString(partData))
                .setCseMultiPartContext(ctx));
        if (!partOutcome.has_value()) {
            auto& e = partOutcome.error();
            std::cerr << "UploadPart " << (i + 1) << " fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage() << std::endl;
            return 1;
        }

        oss::models::Part part;
        part.partNumber = i + 1;
        part.eTag = partOutcome->getETag();
        parts.push_back(part);
        std::cout << "  Part " << (i + 1) << " uploaded" << std::endl;
    }

    // 3. Complete multipart upload
    oss::models::CompleteMultipartUpload cmu;
    cmu.setParts(parts);
    auto completeOutcome = client.completeMultipartUpload(
        oss::models::CompleteMultipartUploadRequest()
            .setBucket(bucket)
            .setKey(key)
            .setUploadId(uploadId)
            .setCompleteMultipartUpload(cmu));
    if (!completeOutcome.has_value()) {
        auto& e = completeOutcome.error();
        std::cerr << "CompleteMultipartUpload fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    std::cout << "Complete, requestId: " << completeOutcome->getRequestId() << std::endl;

    // 4. Download and verify
    auto getOutcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key));
    if (!getOutcome.has_value()) {
        auto& e = getOutcome.error();
        std::cerr << "GetObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << getOutcome->getBody()->rdbuf();
    std::string got = ss.str();
    if (got == content) {
        std::cout << "Verified: decrypted content matches original ("
                  << got.size() << " bytes)" << std::endl;
    } else {
        std::cerr << "Mismatch: expected " << content.size()
                  << " bytes, got " << got.size() << std::endl;
        return 1;
    }

    return 0;
}
