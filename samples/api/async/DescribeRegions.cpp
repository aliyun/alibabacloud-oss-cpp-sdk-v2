// Demonstrates: Async query endpoints of all or specific regions.
#include "SampleConfig.h"

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty())
        sample::printUsageAndExit(argv[0], "");

    std::string regions;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--regions" && i + 1 < argc)
            regions = argv[++i];
    }

    auto client = sample::createAsyncClient(args);

    auto request = oss::models::DescribeRegionsRequest();
    if (!regions.empty())
        request.setRegions(regions);

    auto future = client->asyncCall(request);

    auto outcome = future.get();
    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "DescribeRegions fail"
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

    for (const auto& rg : result.getRegionInfoList().regionInfos) {
        std::cout << "Region: " << rg.region.value_or("")
                  << ", InternetEndpoint: " << rg.internetEndpoint.value_or("")
                  << ", InternalEndpoint: " << rg.internalEndpoint.value_or("") << std::endl;
    }
    return 0;
}
