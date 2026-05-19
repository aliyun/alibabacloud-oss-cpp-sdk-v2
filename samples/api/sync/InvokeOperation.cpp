// Demonstrates: Call any API using the low-level invokeOperation interface.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    // Example: use invokeOperation to call GetBucketLocation
    oss::OperationInput input;
    input.opName = "GetBucketLocation";
    input.method = "GET";
    input.bucket = args.bucket;
    input.parameters["location"] = "";

    auto result = client.invokeOperation(input);

    if (std::holds_alternative<oss::OperationError>(result)) {
        auto& e = std::get<oss::OperationError>(result);
        std::cerr << "InvokeOperation fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }

    auto& output = std::get<oss::OperationOutput>(result);
    std::cout << "status code: " << output.statusCode
              << ", requestId: " << output.headers.at("x-oss-request-id") << std::endl;
    return 0;
}
