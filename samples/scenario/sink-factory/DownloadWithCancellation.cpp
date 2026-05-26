// Demonstrates: Cancel a download mid-flight, keeping the partially received data.
//
// Combines CancellationToken with SinkFactory. When the token is canceled during
// transfer, the SDK aborts the HTTP stream. Data already written to the ByteWriter
// is still valid and usable -- this enables "download first N bytes" or timeout
// patterns for large files.
//
// Usage:
//   ./DownloadWithCancellation --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, key;
    int cancelAfterMs = 100;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--key" && i + 1 < argc) key = argv[++i];
        else if (a == "--cancel-after-ms" && i + 1 < argc) cancelAfterMs = std::stoi(argv[++i]);
    }
    if (region.empty() || bucket.empty() || key.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket> --key <key>"
                  << " [--endpoint <endpoint>] [--cancel-after-ms <ms>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = oss::OSSClient(conf);

    // Track how many bytes we received before cancellation
    std::shared_ptr<oss::MemoryWriter> memWriter;
    std::string localFile = key + ".partial";
    std::shared_ptr<std::ofstream> fileStream = nullptr;

    oss::SinkFactory factory;
    factory.isOneShot = true;
    factory.supplier = [&](std::int64_t, const oss::HeaderCollection&) -> std::shared_ptr<oss::ByteWriter> {
        fileStream = std::make_shared<std::ofstream>(localFile, std::ios::binary | std::ios::trunc);
        return std::make_shared<oss::OStreamWriter>(fileStream);
    };

    // Create cancellation token
    auto cts = oss::CancellationTokenSource::create();

    // Spawn a thread that cancels after a delay
    std::thread canceler([&cts, cancelAfterMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(cancelAfterMs));
        std::cout << "Canceling download after " << cancelAfterMs << "ms..." << std::endl;
        cts->cancel();
    });

    oss::OperationOptions opts;
    opts.cancellationToken = cts->getToken();

    std::cout << "Downloading " << key << " (will cancel after " << cancelAfterMs << "ms)..."
              << std::endl;

    auto outcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setSinkFactory(factory),
        &opts);

    canceler.join();

    // Flush and close the file to ensure partial data is persisted
    if (fileStream) {
        fileStream->flush();
        fileStream.reset();
    }

    if (outcome.has_value()) {
        // Download completed before cancellation fired
        std::cout << "Download completed before cancellation"
                  << ", status: " << outcome.value().getStatusCode()
                  << ", file: " << localFile << std::endl;
    } else {
        auto& e = outcome.error();
        std::cout << "Download canceled (expected)"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;

        // The partial file is still valid and contains data received so far
        std::ifstream check(localFile, std::ios::binary | std::ios::ate);
        if (check.is_open()) {
            auto partialSize = check.tellg();
            std::cout << "Partial data saved: " << partialSize << " bytes"
                      << ", file: " << localFile << std::endl;
        }
    }

    return 0;
}
