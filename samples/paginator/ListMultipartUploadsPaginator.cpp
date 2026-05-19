// Demonstrates: Paginate through all ongoing multipart uploads in a bucket.
#include "SampleConfig.h"

#include "alibabacloud/oss2/Paginator.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto paginator = oss::makePaginator(client,
        oss::models::ListMultipartUploadsRequest()
            .setBucket(args.bucket)
            .setMaxUploads(100));

    int pageNum = 0;
    int totalUploads = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ListMultipartUploads fail"
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
        for (const auto& u : result.getUploads()) {
            std::cout << "  key: " << u.key
                      << ", uploadId: " << u.uploadId
                      << ", initiated: " << u.initiated << std::endl;
            totalUploads++;
        }
    }
    std::cout << "Total uploads: " << totalUploads
              << ", pages: " << pageNum << std::endl;
    return 0;
}
