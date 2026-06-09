// Demonstrates: Process an object (e.g. image resize, watermark) and save the result.
#include "SampleConfig.h"
#include "alibabacloud/oss2/utils/Base64Utils.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    std::string targetKey;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--target-key" && i + 1 < argc)
            targetKey = argv[++i];
    }
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || targetKey.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key> --target-key <target-key>");

    auto client = sample::createClient(args);

    std::string process = "image/resize,w_100|sys/saveas,o_"
        + oss::utils::Base64Encode(targetKey) + ",b_"
        + oss::utils::Base64Encode(args.bucket);

    auto outcome = client.processObject(
        oss::models::ProcessObjectRequest()
            .setBucket(args.bucket)
            .setKey(args.key)
            .setProcess(process));
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ProcessObject fail"
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
              << ", body: " << result.getBody() << std::endl;
    return 0;
}
