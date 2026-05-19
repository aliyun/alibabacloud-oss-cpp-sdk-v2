// Demonstrates: Paginate through all object versions including delete markers.
#include "SampleConfig.h"

#include "alibabacloud/oss2/Paginator.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto paginator = oss::makePaginator(client,
        oss::models::ListObjectVersionsRequest()
            .setBucket(args.bucket)
            .setMaxKeys(100));

    int pageNum = 0;
    int totalVersions = 0;
    int totalDeleteMarkers = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
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
        pageNum++;
        std::cout << "Page " << pageNum
                  << ", isTruncated: " << result.getIsTruncated() << std::endl;
        for (const auto& ver : result.getVersions()) {
            std::cout << "  Version: " << ver.key
                      << ", versionId: " << ver.versionId
                      << ", size: " << ver.size
                      << ", isLatest: " << ver.isLatest.value_or(false) << std::endl;
            totalVersions++;
        }
        for (const auto& dm : result.getDeleteMarkers()) {
            std::cout << "  DeleteMarker: " << dm.key
                      << ", versionId: " << dm.versionId
                      << ", isLatest: " << dm.isLatest.value_or(false) << std::endl;
            totalDeleteMarkers++;
        }
    }
    std::cout << "Total versions: " << totalVersions
              << ", deleteMarkers: " << totalDeleteMarkers
              << ", pages: " << pageNum << std::endl;
    return 0;
}
