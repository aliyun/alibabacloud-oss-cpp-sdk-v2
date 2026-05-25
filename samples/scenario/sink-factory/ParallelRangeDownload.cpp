// Demonstrates: Parallel range download using asyncCall and MemoryWriter.
//
// Splits an object into N parts, issues concurrent asyncCall requests with
// different Range headers, each writing to a MemoryWriter at the corresponding
// buffer offset. All futures are collected and awaited. This is the classic
// pattern for accelerating large file downloads.
//
// Usage:
//   ./ParallelRangeDownload --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <vector>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket, key;
    int numParts = 4;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--key" && i + 1 < argc) key = argv[++i];
        else if (a == "--parts" && i + 1 < argc) numParts = std::stoi(argv[++i]);
    }
    if (region.empty() || bucket.empty() || key.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " --region <region> --bucket <bucket> --key <key>"
                  << " [--endpoint <endpoint>] [--parts <N>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = std::make_shared<oss::OSSAsyncClient>(conf);

    // HEAD to get total size
    auto headFuture = client->asyncCall(
        oss::models::HeadObjectRequest()
            .setBucket(bucket)
            .setKey(key));
    auto headOutcome = headFuture.get();
    if (!headOutcome.has_value()) {
        auto& e = headOutcome.error();
        std::cerr << "HeadObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    std::int64_t totalSize = headOutcome.value().getContentLength();
    std::cout << "Object size: " << totalSize << " bytes, parts: " << numParts << std::endl;

    if (totalSize == 0) {
        std::cout << "Object is empty, nothing to download." << std::endl;
        return 0;
    }

    // Allocate a single contiguous buffer for the entire object
    auto buffer = std::make_shared<std::vector<std::uint8_t>>(static_cast<std::size_t>(totalSize));

    // Calculate part ranges and issue async requests
    std::int64_t partSize = (totalSize / numParts) & ~0xFFFLL;
    if (partSize == 0) partSize = 4096;
    numParts = static_cast<int>((totalSize + partSize - 1) / partSize);

    auto startTime = std::chrono::steady_clock::now();

    std::vector<std::future<oss::GetObjectOutcome>> futures;
    for (int i = 0; i < numParts; i++) {
        std::int64_t start = i * partSize;
        std::int64_t end = (i == numParts - 1) ? (totalSize - 1) : ((i + 1) * partSize - 1);
        std::size_t offset = static_cast<std::size_t>(start);
        std::size_t length = static_cast<std::size_t>(end - start + 1);

        // Each SinkFactory captures by value: shared_ptr to buffer + offset + length
        oss::SinkFactory factory;
        factory.isOneShot = false;
        factory.supplier = [buffer, offset, length](std::int64_t) -> std::shared_ptr<oss::ByteWriter> {
            return std::make_shared<oss::MemoryWriter>(buffer->data() + offset, length);
        };

        std::string rangeHeader = "bytes=" + std::to_string(start) + "-" + std::to_string(end);

        futures.push_back(client->asyncCall(
            oss::models::GetObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setRange(rangeHeader)
                .setRangeBehavior("standard")
                .setSinkFactory(factory)));
    }

    // Wait for all parts to complete
    bool allSuccess = true;
    for (int i = 0; i < numParts; i++) {
        auto outcome = futures[i].get();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "Part " << i << " failed"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage() << std::endl;
            allSuccess = false;
        }
    }

    auto elapsed = std::chrono::steady_clock::now() - startTime;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    if (!allSuccess) {
        std::cerr << "Download failed." << std::endl;
        return 1;
    }

    std::cout << "Download complete: " << totalSize << " bytes in " << ms << "ms"
              << " (" << numParts << " parallel parts)" << std::endl;

    // Print first 100 bytes as preview
    std::size_t preview = std::min<std::size_t>(static_cast<std::size_t>(totalSize), 100);
    std::cout << "Preview: "
              << std::string(reinterpret_cast<char*>(buffer->data()), preview)
              << (static_cast<std::size_t>(totalSize) > preview ? "..." : "") << std::endl;

    return 0;
}
