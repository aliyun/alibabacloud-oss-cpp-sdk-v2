// Demonstrates: Async upload a part by copying from an existing object.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string uploadId, sourceBucket, sourceKey;
    int partNumber = 1;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--upload-id" && i + 1 < argc)
            uploadId = argv[++i];
        else if (a == "--part-number" && i + 1 < argc)
            partNumber = std::stoi(argv[++i]);
        else if (a == "--source-bucket" && i + 1 < argc)
            sourceBucket = argv[++i];
        else if (a == "--source-key" && i + 1 < argc)
            sourceKey = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || uploadId.empty() || sourceBucket.empty() ||
        sourceKey.empty())
        sample::printUsageAndExit(argv[0], " --bucket <b> --key <k> --upload-id <id> --source-bucket <sb> --source-key <sk>");

    auto client = sample::createAsyncClient(args);

    auto future = client->asyncCall(
        oss::models::UploadPartCopyRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setUploadId(uploadId)
            .setPartNumber(partNumber)
            .setSourceBucket(sourceBucket)
            .setSourceKey(sourceKey));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "UploadPartCopy fail"
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
              << ", eTag: " << result.getETag()
              << ", lastModified: " << result.getLastModified() << std::endl;
    return 0;
}
