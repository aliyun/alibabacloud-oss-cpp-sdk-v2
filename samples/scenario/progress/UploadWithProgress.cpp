// Demonstrates: Monitor upload progress using ProgressCallback on PutObject.
//
// ProgressCallback is invoked during data transfer with:
//   - increment:   bytes sent since the last callback invocation
//   - transferred: cumulative bytes sent so far
//   - total:       total content length (-1 if unknown)
//
// Usage:
//   ./UploadWithProgress --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = oss::OSSClient(conf);

    // Generate a ~1 MB payload to make progress callbacks visible
    std::string data(1024 * 1024, 'A');
    std::string key = "progress-demo/upload-with-progress.dat";

    // Set up progress callback
    oss::ProgressCallback progress;
    progress.callback = [](std::size_t increment, std::size_t transferred,
                           std::int64_t total, std::uintptr_t) {
        if (total > 0) {
            int pct = static_cast<int>(transferred * 100 / static_cast<std::size_t>(total));
            std::cout << "\r  progress: " << transferred << " / " << total
                      << " bytes (" << pct << "%)" << std::flush;
        } else {
            std::cout << "\r  progress: " << transferred << " bytes (total unknown)"
                      << std::flush;
        }
    };

    std::cout << "Uploading " << data.size() << " bytes to " << key << " ..." << std::endl;

    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(oss::RequestBody::fromString(data))
            .setProgressCallback(progress));

    std::cout << std::endl;

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    std::cout << "PutObject done"
              << ", status: " << outcome.value().getStatusCode()
              << ", requestId: " << outcome.value().getRequestId() << std::endl;

    // Cleanup
    client.deleteObject(
        oss::models::DeleteObjectRequest()
            .setBucket(bucket)
            .setKey(key));

    return 0;
}
