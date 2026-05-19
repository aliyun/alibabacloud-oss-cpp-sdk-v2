// Demonstrates: List all ongoing multipart uploads in a bucket.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.listMultipartUploads(
        oss::models::ListMultipartUploadsRequest()
            .setBucket(args.bucket)
            .setMaxUploads(100));
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
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId()
              << ", bucket: " << result.getBucket()
              << ", isTruncated: " << result.getIsTruncated() << std::endl;

    for (const auto& u : result.getUploads()) {
        std::cout << "Upload: key=" << u.key
                  << ", uploadId=" << u.uploadId
                  << ", initiated=" << u.initiated << std::endl;
    }
    return 0;
}
