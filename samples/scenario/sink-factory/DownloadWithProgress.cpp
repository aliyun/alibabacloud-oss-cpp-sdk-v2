// Demonstrates: Monitor download progress using SinkFactory with ProgressWriteObserver.
//
// Unlike upload progress (set via setProgressCallback on the request), download
// progress is tracked by attaching a ProgressWriteObserver to ObservableWriter
// through SinkFactory.
//
// Usage:
//   ./DownloadWithProgress --region <region> --bucket <bucket> --key <key>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/io/ByteWriter.h"

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

    auto client = oss::OSSClient(conf);

    // Set up progress callback
    oss::ProgressCallback progress;
    progress.callback = [](std::size_t, std::size_t transferred,
                           std::int64_t total, std::uintptr_t) {
        if (total > 0) {
            int pct = static_cast<int>(transferred * 100 / static_cast<std::size_t>(total));
            std::cout << "\r  downloading: " << transferred << " / " << total
                      << " bytes (" << pct << "%)" << std::flush;
        } else {
            std::cout << "\r  downloading: " << transferred << " bytes" << std::flush;
        }
    };

    // SinkFactory: creates an ObservableWriter with file output + progress observer
    std::string localFile = key + ".download";
    oss::SinkFactory factory;
    factory.isOneShot = false;
    factory.supplier = [&](std::int64_t contentLength) -> std::shared_ptr<oss::ByteWriter> {
        auto progressObs = std::make_shared<oss::ProgressWriteObserver>(progress, contentLength);
        auto file = std::make_shared<std::ofstream>(localFile, std::ios::binary | std::ios::trunc);
        auto writer = std::make_shared<oss::OStreamWriter>(file);
        return std::make_shared<oss::ObservableWriter>(writer, progressObs);
    };

    std::cout << "Downloading " << key << " to " << localFile << " ..." << std::endl;

    auto outcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setSinkFactory(factory));

    std::cout << std::endl;

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
              << ", file: " << localFile << std::endl;

    return 0;
}
