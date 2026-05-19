// Demonstrates: Create a symbolic link to a target object.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string target;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--target" && i + 1 < argc)
            target = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || target.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <symlink-key> --target <target-key>");

    auto client = sample::createClient(args);

    auto outcome = client.putSymlink(
        oss::models::PutSymlinkRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setSymlinkTarget(target));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "PutSymlink fail"
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
