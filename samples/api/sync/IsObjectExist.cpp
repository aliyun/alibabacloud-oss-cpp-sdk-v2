// Demonstrates: Check whether an object exists in a bucket.
//
// Return values:
//   outcome.value() == true  : the object exists
//   outcome.value() == false : the object does not exist
//   outcome.has_value() == false (error) : unable to determine;
//       this does NOT imply the object exists or does not exist.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createClient(args);

    auto outcome = client.isObjectExist(args.bucket, args.key);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "IsObjectExist error, unable to determine existence"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    std::cout << "Object \"" << args.key << "\" "
              << (outcome.value() ? "exists" : "does not exist") << std::endl;
    return 0;
}
