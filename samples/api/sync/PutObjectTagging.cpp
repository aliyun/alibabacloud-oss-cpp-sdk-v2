// Demonstrates: Add or update tags on an object.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    oss::models::TagSet tagSet;
    tagSet.tags.push_back(oss::models::Tag{"env", "test"});
    tagSet.tags.push_back(oss::models::Tag{"project", "demo"});

    oss::models::Tagging tagging;
    tagging.tagSet = tagSet;

    auto outcome = client.putObjectTagging(
        oss::models::PutObjectTaggingRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setTagging(tagging));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutObjectTagging fail"
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
              << ", versionId: " << result.getVersionId() << std::endl;
    return 0;
}
