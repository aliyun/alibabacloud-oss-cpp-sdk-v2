// Demonstrates: Upload an encrypted object and download it with transparent decryption.
//
// Usage:
//   ./sample_encryption_PutGetObject --region <region> --bucket <bucket> --key <key> \
//       --public-key-file <path> --private-key-file <path>
#include "SampleConfig.h"

#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/RsaMasterCipher.h"

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

    // Upload
    std::string data = "Hello, client-side encryption!";
    auto putOutcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(oss::RequestBody::fromString(data)));
    if (!putOutcome.has_value()) {
        auto& e = putOutcome.error();
        std::cerr << "PutObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    std::cout << "PutObject done, requestId: " << putOutcome->getRequestId() << std::endl;

    // Download
    auto getOutcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key));
    if (!getOutcome.has_value()) {
        auto& e = getOutcome.error();
        std::cerr << "GetObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << getOutcome->getBody()->rdbuf();
    std::cout << "GetObject done, content: " << ss.str() << std::endl;

    return 0;
}
