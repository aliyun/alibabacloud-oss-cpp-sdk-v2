// Demonstrates: List object versions including delete markers.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.listObjectVersions(
        oss::models::ListObjectVersionsRequest()
            .setBucket(args.bucket)
            .setMaxKeys(100));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectVersions fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", isTruncated: " << result.getIsTruncated() << std::endl;

    for (const auto& ver : result.getVersions()) {
        std::cout << "Version: " << ver.key
                  << ", versionId: " << ver.versionId
                  << ", size: " << ver.size
                  << ", lastModified: " << ver.lastModified
                  << ", isLatest: " << ver.isLatest.value_or(false) << std::endl;
    }
    for (const auto& dm : result.getDeleteMarkers()) {
        std::cout << "DeleteMarker: " << dm.key
                  << ", versionId: " << dm.versionId
                  << ", lastModified: " << dm.lastModified
                  << ", isLatest: " << dm.isLatest.value_or(false) << std::endl;
    }
    return 0;
}
