// Demonstrates: Generate a presigned URL for UploadPart.
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
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --upload-id <uploadId>");

    auto client = sample::createClient(args);

    oss::models::PresignOptions options;
    options.setExpirationDuration(std::chrono::seconds(3600));

    auto outcome = client.presign(
        oss::models::UploadPartRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setPartNumber(1),
        &options);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "Presign(UploadPart) fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = outcome.value();
    std::cout << "method: " << result.getMethod()
              << ", expiration: " << result.getExpiration()
              << ", url: " << result.getUrl() << std::endl;
    for (const auto& h : result.getSignedHeaders()) {
        std::cout << "  header: " << h.first << " = " << h.second << std::endl;
    }
    return 0;
}
