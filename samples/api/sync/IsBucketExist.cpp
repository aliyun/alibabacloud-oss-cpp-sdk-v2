// Demonstrates: Check whether a bucket exists.
//
// Return values:
//   outcome.value() == true  : the bucket exists
//   outcome.value() == false : the bucket does not exist
//   outcome.has_value() == false (error) : unable to determine;
//       this does NOT imply the bucket exists or does not exist.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket>");

    auto client = sample::createClient(args);

    auto outcome = client.isBucketExist(args.bucket);
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "IsBucketExist error, unable to determine existence"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }
    std::cout << "Bucket \"" << args.bucket << "\" "
              << (outcome.value() ? "exists" : "does not exist") << std::endl;
    return 0;
}
