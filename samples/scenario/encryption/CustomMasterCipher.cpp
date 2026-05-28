// Demonstrates: Implementing a custom MasterCipher for OSSEncryptionClient.
//
// WARNING: The XOR cipher used here is NOT secure and is FOR DEMONSTRATION ONLY.
// DO NOT use this in production. This sample only shows how to implement the
// MasterCipher interface. Replace the XOR logic with a real key management
// system (e.g. KMS) for production use.
//
// Usage:
//   ./CustomMasterCipher --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/crypto/OSSEncryptionClient.h"
#include "alibabacloud/oss2/crypto/MasterCipher.h"
#include "alibabacloud/oss2/crypto/Error.h"

#include <array>
#include <iostream>
#include <sstream>

namespace oss = alibabacloud::oss2;

// A toy MasterCipher that XOR-wraps the data key with a fixed 32-byte secret.
// NOT SECURE -- for demonstration only. DO NOT use in production.
class XorMasterCipher : public oss::crypto::MasterCipher {
  public:
    explicit XorMasterCipher(std::array<unsigned char, 32> secret)
        : secret_(secret) {}

    oss::crypto::MasterCipherResult encrypt(const std::string& plaintext) const override {
        return xorTransform(plaintext);
    }

    oss::crypto::MasterCipherResult decrypt(const std::string& ciphertext) const override {
        return xorTransform(ciphertext);
    }

    std::string getWrapAlgorithm() const override {
        return "custom/xor-demo";
    }

    std::string getMatDesc() const override {
        return R"({"type":"xor-demo","version":"1"})";
    }

  private:
    std::string xorTransform(const std::string& input) const {
        std::string output(input.size(), '\0');
        for (size_t i = 0; i < input.size(); i++) {
            output[i] = static_cast<char>(
                static_cast<unsigned char>(input[i]) ^ secret_[i % secret_.size()]);
        }
        return output;
    }

    std::array<unsigned char, 32> secret_;
};

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    // Create a custom MasterCipher
    std::array<unsigned char, 32> secret{};
    for (size_t i = 0; i < secret.size(); i++) {
        secret[i] = static_cast<unsigned char>(i + 0x42);
    }
    auto masterCipher = std::make_shared<XorMasterCipher>(secret);

    oss::crypto::EncryptionConfiguration encConfig;
    encConfig.masterCipher = masterCipher;
    oss::OSSEncryptionClient client(conf, std::move(encConfig));

    std::string key = "custom-cipher-demo/test-object";
    std::string data = "Hello from custom MasterCipher!";

    // Upload (encrypted)
    auto putOutcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(oss::RequestBody::FromString(data)));
    if (!putOutcome.has_value()) {
        auto& e = putOutcome.error();
        std::cerr << "PutObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    std::cout << "PutObject done, requestId: " << putOutcome->getRequestId() << std::endl;

    // Download (decrypted)
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
    std::cout << "GetObject done, content: " << ss.str() << std::endl;

    // Cleanup
    client.unwrap().deleteObject(
        oss::models::DeleteObjectRequest()
            .setBucket(bucket)
            .setKey(key));

    return 0;
}
