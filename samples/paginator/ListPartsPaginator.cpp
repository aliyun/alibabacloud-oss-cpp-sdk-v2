// Demonstrates: Paginate through all parts of a multipart upload.
#include "SampleConfig.h"

#include "alibabacloud/oss2/Paginator.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string uploadId;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--upload-id" && i + 1 < argc)
            uploadId = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || uploadId.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --upload-id <id>");

    auto client = sample::createClient(args);

    auto paginator = oss::makePaginator(client,
        oss::models::ListPartsRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId));

    int pageNum = 0;
    int totalParts = 0;
    while (paginator.hasNext()) {
        auto outcome = paginator.nextPage();
        if (!outcome.has_value()) {
            auto& e = outcome.error();
            std::cerr << "ListParts fail"
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
        for (const auto& p : result.getParts()) {
            std::cout << "  partNumber: " << p.partNumber
                      << ", eTag: " << p.eTag
                      << ", size: " << p.size
                      << ", lastModified: " << p.lastModified << std::endl;
            totalParts++;
        }
    }
    std::cout << "Total parts: " << totalParts
              << ", pages: " << pageNum << std::endl;
    return 0;
}
