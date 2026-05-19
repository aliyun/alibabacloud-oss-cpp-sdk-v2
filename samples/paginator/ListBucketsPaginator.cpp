// Demonstrates: Paginate through all buckets owned by the current user.
#include "SampleConfig.h"

#include "alibabacloud/oss2/Paginator.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty())
        sample::printUsageAndExit(argv[0], "");

    auto client = sample::createClient(args);

    auto paginator = oss::makePaginator(client, oss::models::ListBucketsRequest());

    int pageNum = 0;
    int totalBuckets = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ListBuckets fail"
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
        for (const auto& b : result.getBuckets()) {
            std::cout << "  " << b.name
                      << ", location: " << b.location
                      << ", storageClass: " << b.storageClass << std::endl;
            totalBuckets++;
        }
    }
    std::cout << "Total buckets: " << totalBuckets
              << ", pages: " << pageNum << std::endl;
    return 0;
}
