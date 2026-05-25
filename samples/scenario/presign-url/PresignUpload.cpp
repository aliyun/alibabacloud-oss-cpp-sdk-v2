// Demonstrates: Upload a file using a presigned URL.
//
// Interaction flow:
//
//   Client                     App Server (your backend)           OSS
//     |                               |                              |
//     |  1. request upload URL        |                              |
//     |------------------------------>|                              |
//     |                               |  2. presign(PutObject)       |
//     |                               |  (local signing, no network) |
//     |  3. return URL + headers      |                              |
//     |<------------------------------|                              |
//     |                                                              |
//     |  4. HTTP PUT with signed headers                             |
//     |------------------------------------------------------------->|
//     |                                            200 OK            |
//     |<-------------------------------------------------------------|
//
// This sample starts an App Server (httplib) and a Client in the same process
// to demonstrate the full interaction.  In production they are separate apps.
//
// Usage:
//   ./PresignUpload --region <region> --bucket <bucket> [--endpoint <endpoint>]

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <httplib.h>

#include <iostream>
#include <string>
#include <thread>

namespace oss = alibabacloud::oss2;

static std::pair<std::string, std::string> splitUrl(const std::string& url) {
    auto schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) return {"http://" + url, "/"};
    auto pathStart = url.find('/', schemeEnd + 3);
    if (pathStart == std::string::npos) return {url, "/"};
    return {url.substr(0, pathStart), url.substr(pathStart)};
}

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

    // ===== App Server setup =====

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    auto ossClient = oss::OSSClient(conf);

    httplib::Server svr;

    // App Server route: client requests a presigned upload URL
    //   GET /presign/upload?key=xxx
    //   Response body (line-based):
    //     Line 1: presigned URL
    //     Line 2+: signed-header-name:value (one per line)
    svr.Get("/presign/upload", [&](const httplib::Request& req, httplib::Response& res) {
        auto key = req.get_param_value("key");
        if (key.empty()) {
            res.status = 400;
            res.set_content("missing 'key' parameter", "text/plain");
            return;
        }

        auto putReq = oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key);
        // Include Content-Type in the signature. Some HTTP clients add a
        // default Content-Type automatically; if it differs from the one
        // used during signing, OSS will return a SignatureDoesNotMatch error.
        // You can also add other custom headers here to include them in the
        // signature (e.g. x-oss-meta-*, Content-Disposition, etc.).
        putReq.addHeader("Content-Type", "text/plain; charset=utf-8");

        oss::models::PresignOptions opts;
        opts.setExpirationDuration(std::chrono::seconds(3600));

        auto outcome = ossClient.presign(putReq, &opts);
        if (!outcome.has_value()) {
            res.status = 500;
            res.set_content("presign failed: " + outcome.error().getMessage(), "text/plain");
            return;
        }

        auto& result = outcome.value();
        std::string body = result.getUrl() + "\n";
        for (const auto& h : result.getSignedHeaders()) {
            body += h.first + ":" + h.second + "\n";
        }
        res.set_content(body, "text/plain");
    });

    int port = svr.bind_to_any_port("127.0.0.1");
    std::thread serverThread([&svr]() { svr.listen_after_bind(); });
    svr.wait_until_ready();
    std::cout << "[App Server] listening on http://127.0.0.1:" << port << std::endl;

    // ===== Client =====

    int exitCode = 0;
    std::string key = "presign-demo/upload-sample.txt";

    // Step 1: request presigned URL from App Server
    std::cout << "\n[Client] requesting presigned upload URL..." << std::endl;
    httplib::Client appClient("http://127.0.0.1:" + std::to_string(port));
    auto presignRes = appClient.Get("/presign/upload?key=" + key);
    if (!presignRes || presignRes->status != 200) {
        std::cerr << "[Client] failed to get presigned URL from App Server" << std::endl;
        svr.stop();
        serverThread.join();
        return 1;
    }

    // Parse response: first line is URL, remaining lines are signed headers
    std::string presignBody = presignRes->body;
    auto firstNl = presignBody.find('\n');
    std::string presignedUrl = presignBody.substr(0, firstNl);
    httplib::Headers signedHeaders;
    std::string headersPart = presignBody.substr(firstNl + 1);
    std::istringstream iss(headersPart);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            signedHeaders.emplace(line.substr(0, colon), line.substr(colon + 1));
        }
    }
    std::cout << "[Client] got presigned URL: " << presignedUrl << std::endl;

    // Step 2: upload file directly to OSS using the presigned URL
    std::string content = "Hello OSS! Uploaded via presigned URL.";

    auto [base, path] = splitUrl(presignedUrl);
    httplib::Client ossHttp(base);
    ossHttp.set_connection_timeout(std::chrono::seconds(10));
    ossHttp.set_read_timeout(std::chrono::seconds(30));
    ossHttp.set_follow_location(true);
    ossHttp.set_url_encode(false);

    std::cout << "[Client] uploading to OSS via HTTP PUT..." << std::endl;
    auto putRes = ossHttp.Put(path, signedHeaders, content, "text/plain; charset=utf-8");
    if (putRes && putRes->status == 200) {
        std::cout << "[Client] upload succeeded! HTTP " << putRes->status << std::endl;
    } else {
        std::cerr << "[Client] upload failed" << std::endl;
        exitCode = 1;
    }

    // Cleanup
    ossClient.deleteObject(
        oss::models::DeleteObjectRequest().setBucket(bucket).setKey(key));

    svr.stop();
    serverThread.join();
    return exitCode;
}
