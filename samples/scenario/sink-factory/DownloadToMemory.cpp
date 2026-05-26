// Demonstrates: Zero-copy download into a pre-allocated memory buffer using MemoryWriter.
//
// MemoryWriter writes response data directly into the user-provided buffer,
// avoiding extra copies compared to reading from a std::iostream body.
//
// Usage:
//   ./DownloadToMemory --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <iostream>
#include <string>
#include <vector>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, key;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--key" && i + 1 < argc) key = argv[++i];
    }
    if (region.empty() || bucket.empty() || key.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket> --key <key>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = oss::OSSClient(conf);

    // HEAD the object to know buffer size
    auto headOutcome = client.headObject(
        oss::models::HeadObjectRequest()
            .setBucket(bucket)
            .setKey(key));
    if (!headOutcome.has_value()) {
        auto& e = headOutcome.error();
        std::cerr << "HeadObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    std::size_t contentLength = static_cast<std::size_t>(headOutcome.value().getContentLength());
    std::cout << "Object size: " << contentLength << " bytes" << std::endl;

    // Pre-allocate buffer
    std::vector<std::uint8_t> buffer(contentLength);

    // SinkFactory with MemoryWriter -- zero-copy into buffer
    std::shared_ptr<oss::MemoryWriter> memWriter;
    oss::SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [&](std::int64_t, const oss::HeaderCollection&) -> std::shared_ptr<oss::ByteWriter> {
        memWriter = std::make_shared<oss::MemoryWriter>(buffer.data(), buffer.size());
        return memWriter;
    };

    auto outcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setSinkFactory(factory));

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "GetObject done"
              << ", status: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", bytes received: " << memWriter->written() << std::endl;

    // Print first 100 bytes as preview
    std::size_t preview = std::min<std::size_t>(memWriter->written(), 100);
    std::cout << "Preview: "
              << std::string(reinterpret_cast<char*>(buffer.data()), preview)
              << (memWriter->written() > preview ? "..." : "") << std::endl;

    return 0;
}
