// Demonstrates: Resumable download that recovers from network interruptions using Range requests.
//
// On each attempt the SinkFactory opens the local file in append mode and sets
// the Range header to skip already-received bytes. If the download is interrupted,
// the loop retries from the last written offset until the file is complete.
//
// Usage:
//   ./ResumableDownload --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

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

    // HEAD to get total size and ETag for consistency check
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

    std::int64_t totalSize = headOutcome.value().getContentLength();
    std::string etag = headOutcome.value().getETag();
    std::cout << "Object size: " << totalSize << " bytes, ETag: " << etag << std::endl;

    std::string localFile = key + ".download";
    std::int64_t offset = 0;

    // Check if a partial file already exists from a previous attempt
    {
        std::ifstream existing(localFile, std::ios::binary | std::ios::ate);
        if (existing.is_open()) {
            offset = static_cast<std::int64_t>(existing.tellg());
            if (offset > 0 && offset < totalSize) {
                std::cout << "Resuming from existing partial file: " << offset << " bytes" << std::endl;
            } else if (offset >= totalSize) {
                std::cout << "File already complete." << std::endl;
                return 0;
            } else {
                offset = 0;
            }
        }
    }

    const int maxRetries = 5;
    int attempt = 0;

    while (offset < totalSize) {
        attempt++;
        if (attempt > maxRetries) {
            std::cerr << "Exceeded max retries (" << maxRetries << "), giving up." << std::endl;
            return 1;
        }

        std::cout << "Attempt " << attempt << ": downloading bytes " << offset
                  << "-" << (totalSize - 1) << std::endl;

        std::shared_ptr<std::ofstream> fileStream = nullptr;

        oss::SinkFactory factory;
        factory.isOneShot = true;
        factory.supplier = [&](std::int64_t) -> std::shared_ptr<oss::ByteWriter> {
            fileStream = std::make_shared<std::ofstream>(
                localFile, std::ios::binary | std::ios::app);
            return std::make_shared<oss::OStreamWriter>(fileStream);
        };

        // Build Range header: "bytes=<offset>-"
        std::string rangeHeader = "bytes=" + std::to_string(offset) + "-";

        auto outcome = client.getObject(
            oss::models::GetObjectRequest()
                .setBucket(bucket)
                .setKey(key)
                .setRange(rangeHeader)
                .setRangeBehavior("standard")
                .setIfMatch(etag)
                .setSinkFactory(factory));

        if (outcome.has_value()) {
            std::cout << "Download complete"
                      << ", status: " << outcome.value().getStatusCode()
                      << ", requestId: " << outcome.value().getRequestId() << std::endl;
            offset = totalSize;
        } else {
            auto& e = outcome.error();

            // If object was modified (ETag mismatch), cannot resume
            if (e.getCode() == "PreconditionFailed") {
                std::cerr << "Object modified during download (ETag mismatch), cannot resume."
                          << std::endl;
                return 1;
            }

            // If supplier was never called, the request failed before receiving response
            // headers (e.g., DNS failure, connection refused). The SDK already retried
            // internally, so this is a persistent failure -- exit immediately.
            if (!fileStream) {
                std::cerr << "GetObject fail (no connection)"
                          << ", code: " << e.getCode()
                          << ", message: " << e.getMessage()
                          << ", ec: " << e.getEC() << std::endl;
                return 1;
            }

            fileStream->flush();
            fileStream.reset();

            // Re-check file size to determine new offset
            std::ifstream check(localFile, std::ios::binary | std::ios::ate);
            std::int64_t newOffset = check.is_open()
                ? static_cast<std::int64_t>(check.tellg()) : offset;

            if (newOffset > offset) {
                std::cout << "  Received " << (newOffset - offset) << " bytes before failure"
                          << std::endl;
                offset = newOffset;
            }

            std::cerr << "  GetObject fail"
                      << ", code: " << e.getCode()
                      << ", message: " << e.getMessage()
                      << ", ec: " << e.getEC() << std::endl;

            // Backoff before retry
            int waitSec = attempt;
            std::cout << "  Retrying in " << waitSec << "s..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(waitSec));
        }
    }

    std::cout << "Downloaded " << totalSize << " bytes to " << localFile << std::endl;
    return 0;
}
