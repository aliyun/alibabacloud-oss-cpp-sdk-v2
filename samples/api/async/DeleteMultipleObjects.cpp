// Demonstrates: Async delete multiple objects in a single request.
#include "SampleConfig.h"
#include <condition_variable>
#include <mutex>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty())
        sample::printUsageAndExit(argv[0], " --bucket <bucket> --key <key>");

    auto client = sample::createAsyncClient(args);

    oss::models::Delete del;
    del.objects.push_back(oss::models::ObjectIdentifier().setKey(args.key));
    del.quiet = false;

    // --- Future pattern (asyncCall) ---
    auto future = client->asyncCall(
        oss::models::DeleteMultipleObjectsRequest()
            .setBucket(args.bucket)
            .setDelete(del));

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "DeleteMultipleObjects fail"
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
    for (const auto& d : result.getDeletedObjects()) {
        std::cout << "Deleted: " << d.key
                  << ", versionId: " << d.versionId.value_or("")
                  << ", deleteMarker: " << d.deleteMarker.value_or(false) << std::endl;
    }
    return 0;
}
