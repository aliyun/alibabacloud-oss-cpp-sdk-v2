// Demonstrates: Abort a multipart upload and delete uploaded parts.
#include "SampleConfig.h"

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

    auto outcome = client.abortMultipartUpload(
        oss::models::AbortMultipartUploadRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "AbortMultipartUpload fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "status code: " << result.getStatusCode()
              << ", requestId: " << result.getRequestId() << std::endl;
    return 0;
}
