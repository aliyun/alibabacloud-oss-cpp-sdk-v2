// Demonstrates: Customize the curl HTTP transport with CurlTransportOptions.
//
// CurlTransportOptions extends HttpTransportOptions with curl-specific settings:
// maxConnections, CA certificates, network interface binding, proxy auth,
// verbose debug output, and a requestInterceptor for arbitrary curl_easy_setopt calls.
//
// Usage:
//   ./CurlCustomConfig --region <region> --bucket <bucket> [--verbose]

#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_HAS_CURL

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportFactory.h"
#include "alibabacloud/oss2/transport/curl/CurlTransportOptions.h"

#include <iostream>
#include <string>

namespace oss = alibabacloud::oss2;

int main(int argc, char* argv[]) {
    std::string region, endpoint, bucket;
    bool verbose = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--region" && i + 1 < argc) region = argv[++i];
        else if (a == "--endpoint" && i + 1 < argc) endpoint = argv[++i];
        else if (a == "--bucket" && i + 1 < argc) bucket = argv[++i];
        else if (a == "--verbose") verbose = true;
    }
    if (region.empty() || bucket.empty()) {
        std::cerr << "Usage: " << argv[0] << " --region <region> --bucket <bucket>"
                  << " [--endpoint <endpoint>] [--verbose]" << std::endl;
        return 1;
    }

    // --- Configure curl transport options ---
    oss::CurlTransportOptions curlOpts;

    // Connection pool: max concurrent connections per host
    curlOpts.maxConnections = 32;

    // Timeouts (milliseconds)
    curlOpts.connectTimeout = 10000;    // 10 seconds
    curlOpts.readWriteTimeout = 30000;  // 30 seconds

    // SSL: custom CA bundle (uncomment to use)
    // curlOpts.caFile = "/etc/ssl/certs/ca-certificates.crt";
    // curlOpts.caPath = "/etc/ssl/certs/";

    // Skip SSL verification (for testing only!)
    // curlOpts.insecureSkipVerify = true;

    // Network interface binding (uncomment to use)
    // curlOpts.networkInterface = "eth0";

    // Proxy configuration (uncomment to use)
    // curlOpts.proxyHost = "http://proxy.example.com";
    // curlOpts.proxyPort = 8080;
    // curlOpts.proxyUserName = "user";
    // curlOpts.proxyPassword = "pass";

    // Verbose debug output — prints curl protocol details to stderr
    if (verbose) {
        curlOpts.enableVerbose = true;
    }

    // Request interceptor: called with the raw CURL* handle before each request.
    // Use this for any curl_easy_setopt call not covered by the options above.
    curlOpts.requestInterceptor = [](void* /*curl*/, const oss::RequestMessage*) {
        // Example: curl_easy_setopt(static_cast<CURL*>(curl), CURLOPT_TCP_KEEPALIVE, 1L);
    };

    // Create the transport and inject into client config
    auto transport = oss::CurlTransportFactory::createHttpTransport(curlOpts);

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
    std::cerr << "This sample requires ALIBABACLOUD_OSS_HAS_CURL." << std::endl;
    return 1;
}
#endif
