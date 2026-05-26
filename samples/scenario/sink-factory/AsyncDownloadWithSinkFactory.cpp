// Demonstrates: Async download with SinkFactory, progress observer, and CRC verification.
//
// SinkFactory works identically with async and sync clients. The key difference
// is that the SinkFactory supplier and observer callbacks are invoked on the
// async IO thread -- never block or do heavy work inside them.
//
// Usage:
//   ./AsyncDownloadWithSinkFactory --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSAsyncClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"
#include "alibabacloud/oss2/utils/CRC64Utils.h"

#include <atomic>
#include <fstream>
#include <iostream>
#include <string>

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

    auto client = std::make_shared<oss::OSSAsyncClient>(conf);

    // Progress tracking -- use shared_ptr<atomic> so lambda captures by value are safe
    auto bytesReceived = std::make_shared<std::atomic<std::size_t>>(0);

    oss::ProgressCallback progress;
    progress.callback = [bytesReceived](std::size_t, std::size_t transferred,
                                        std::int64_t, std::uintptr_t) {
        // NOTE: This callback is invoked on the async IO thread.
        // Keep it lightweight -- no blocking IO or locks here.
        bytesReceived->store(transferred, std::memory_order_relaxed);
    };

    auto crc = std::make_shared<oss::CRC64WriteObserver>();
    std::string localFile = key + ".async-download";

    // Capture by value: shared_ptrs and strings are copied into the lambda,
    // ensuring the factory remains valid even if the caller's scope exits
    // before the async operation completes.
    oss::SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [crc, progress, localFile](std::int64_t contentLength, const oss::HeaderCollection&) -> std::shared_ptr<oss::ByteWriter> {
        crc->reset();
        auto progressObs = std::make_shared<oss::ProgressWriteObserver>(progress, contentLength);
        auto file = std::make_shared<std::ofstream>(localFile, std::ios::binary | std::ios::trunc);
        auto writer = std::make_shared<oss::OStreamWriter>(file);
        return std::make_shared<oss::ObservableWriter>(writer, progressObs, crc);
    };

    std::cout << "Starting async download of " << key << " ..." << std::endl;

    // asyncCall returns a future -- the main thread is free to do other work
    auto future = client->asyncCall(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setSinkFactory(factory));

    // Main thread can do other work here while download proceeds...
    // For this demo, we just wait.
    auto outcome = future.get();

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "GetObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "Download complete"
              << ", status: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", bytes: " << bytesReceived->load()
              << ", file: " << localFile << std::endl;

    // Verify CRC
    if (!result.getHashCrc64ecma().empty()) {
        uint64_t serverCrc = std::stoull(result.getHashCrc64ecma());
        if (serverCrc == crc->crc()) {
            std::cout << "CRC-64 verification: PASSED" << std::endl;
        } else {
            std::cerr << "CRC-64 verification: FAILED" << std::endl;
            return 1;
        }
    }

    return 0;
}
