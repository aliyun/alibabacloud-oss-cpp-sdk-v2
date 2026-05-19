// Demonstrates: Paginate through all objects in a bucket using ListObjects (v1).
#include "SampleConfig.h"

#include "alibabacloud/oss2/Paginator.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto paginator = oss::makePaginator(client,
        oss::models::ListObjectsRequest()
            .setBucket(args.bucket)
            .setMaxKeys(100));

    int pageNum = 0;
    int totalObjects = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ListObjects fail"
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
        for (const auto& obj : result.getContents()) {
            std::cout << "  " << obj.key
                      << ", size: " << obj.size
                      << ", lastModified: " << obj.lastModified << std::endl;
            totalObjects++;
        }
    }
    std::cout << "Total objects: " << totalObjects
              << ", pages: " << pageNum << std::endl;
    return 0;
}
