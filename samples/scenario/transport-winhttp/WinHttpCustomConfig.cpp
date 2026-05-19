// Demonstrates: Customize the WinHTTP transport with WinHttpTransportOptions.
//
// WinHttpTransportOptions provides Windows-native HTTP transport settings:
// maxConnections, proxy authentication, timeouts, and SSL options.
//
// Usage:
//   ./WinHttpCustomConfig --region <region> --bucket <bucket>

#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_HAS_WINHTTP

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportFactory.h"
#include "alibabacloud/oss2/transport/winhttp/WinHttpTransportOptions.h"

#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>]" << std::endl;
        return 1;
    }

    // --- Configure WinHTTP transport options ---
    oss::WinHttpTransportOptions winOpts;

    // Connection pool: max concurrent connections per host
    winOpts.maxConnections = 32;

    // Timeouts (milliseconds)
    winOpts.connectTimeout = 10000;    // 10 seconds
    winOpts.readWriteTimeout = 30000;  // 30 seconds

    // Skip SSL verification (for testing only!)
    // winOpts.insecureSkipVerify = true;

    // Proxy configuration (uncomment to use)
    // winOpts.proxyHost = "http://proxy.example.com";
    // winOpts.proxyPort = 8080;
    // winOpts.proxyUserName = "user";
    // winOpts.proxyPassword = "pass";

    // Create the transport and inject into client config
    auto transport = oss::WinHttpTransportFactory::createHttpTransport(winOpts);

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    conf.httpTransport = transport;

    auto client = oss::OSSClient(conf);

    // Verify: list objects
    auto outcome = client.listObjectsV2(
        oss::models::ListObjectsV2Request()
            .setBucket(bucket)
            .setMaxKeys(5));

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << "ListObjectsV2 fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", requestId: " << e.getRequestId() << std::endl;
        return 1;
    }

    auto& result = outcome.value();
    std::cout << "ListObjectsV2 done, keyCount: " << result.getKeyCount()
              << ", requestId: " << result.getRequestId() << std::endl;
    for (const auto& obj : result.getContents()) {
        std::cout << "  " << obj.key << ", size: " << obj.size << std::endl;
    }
    return 0;
}

#else
#include <iostream>
int main() {
    std::cerr << "This sample requires ALIBABACLOUD_OSS_HAS_WINHTTP (Windows only)." << std::endl;
    return 1;
}
#endif
