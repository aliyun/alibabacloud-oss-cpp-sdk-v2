// Demonstrates: Download a file using a presigned URL.
//
// Interaction flow:
//
//   Client                     App Server (your backend)           OSS
//     |                               |                              |
//     |  1. request download URL      |                              |
//     |------------------------------>|                              |
//     |                               |  2. presign(GetObject)       |
//     |                               |  (local signing, no network) |
//     |  3. return URL                |                              |
//     |<------------------------------|                              |
//     |                                                              |
//     |  4. HTTP GET                                                 |
//     |------------------------------------------------------------->|
//     |                                       200 OK + body          |
//     |<-------------------------------------------------------------|
//
// The presigned URL can be shared via email, chat, or embedded in a web page.
// Anyone with the URL can download the file until it expires.
//
// This sample starts an App Server (httplib) and a Client in the same process
// to demonstrate the full interaction.  In production they are separate apps.
//
// Usage:
//   ./PresignDownload --region <region> --bucket <bucket>
//                     [--endpoint <endpoint>]

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

    // Preparation: upload a test object so there is something to download
    std::string key = "presign-demo/download-sample.txt";
    std::string content = "This file is downloaded via a presigned URL.\n"
                          "No OSS SDK or credentials are needed on the client side.";

    auto putOutcome = ossClient.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(oss::RequestBody::FromString(content)));
    if (!putOutcome.has_value()) {
        auto& e = putOutcome.error();
        std::cerr << "PutObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return 1;
    }
    std::cout << "Test object uploaded: " << key << std::endl;

    // App Server route: client requests a presigned download URL
    //   GET /presign/download?key=xxx
    //   Response body: presigned URL (single line)
    svr.Get("/presign/download", [&](const httplib::Request& req, httplib::Response& res) {
        auto k = req.get_param_value("key");
        if (k.empty()) {
            res.status = 400;
            res.set_content("missing 'key' parameter", "text/plain");
            return;
        }

        oss::models::PresignOptions opts;
        opts.setExpirationDuration(std::chrono::seconds(3600));

        auto outcome = ossClient.presign(
            oss::models::GetObjectRequest()
                .setBucket(bucket)
                .setKey(k),
            &opts);
        if (!outcome.has_value()) {
            res.status = 500;
            res.set_content("presign failed: " + outcome.error().getMessage(), "text/plain");
            return;
        }

        res.set_content(outcome.value().getUrl(), "text/plain");
    });

    int port = svr.bind_to_any_port("127.0.0.1");
    std::thread serverThread([&svr]() { svr.listen_after_bind(); });
    svr.wait_until_ready();
    std::cout << "[App Server] listening on http://127.0.0.1:" << port << std::endl;

    // ===== Client =====

    int exitCode = 0;

    // Step 1: request presigned download URL from App Server
    std::cout << "\n[Client] requesting presigned download URL..." << std::endl;
    httplib::Client appClient("http://127.0.0.1:" + std::to_string(port));
    auto presignRes = appClient.Get("/presign/download?key=" + key);
    if (!presignRes || presignRes->status != 200) {
        std::cerr << "[Client] failed to get presigned URL from App Server" << std::endl;
        svr.stop();
        serverThread.join();
        return 1;
    }

    std::string presignedUrl = presignRes->body;
    std::cout << "[Client] got presigned URL: " << presignedUrl << std::endl;

    // Step 2: download file directly from OSS using the presigned URL
    auto [base, path] = splitUrl(presignedUrl);
    httplib::Client ossHttp(base);
    ossHttp.set_connection_timeout(std::chrono::seconds(10));
    ossHttp.set_read_timeout(std::chrono::seconds(30));
    ossHttp.set_follow_location(true);
    ossHttp.set_url_encode(false);

    std::cout << "[Client] downloading from OSS via HTTP GET..." << std::endl;
    auto getRes = ossHttp.Get(path);
    if (getRes && getRes->status == 200) {
        std::cout << "[Client] download succeeded! HTTP " << getRes->status
                  << "\nDownloaded " << getRes->body.size() << " bytes"
                  << "\nContent:\n" << getRes->body << std::endl;
        if (getRes->body == content) {
            std::cout << "\nContent verification: PASSED" << std::endl;
        } else {
            std::cerr << "\nContent verification: FAILED" << std::endl;
            exitCode = 1;
        }
    } else {
        std::cerr << "[Client] download failed" << std::endl;
        exitCode = 1;
    }

    // Cleanup
    ossClient.deleteObject(
        oss::models::DeleteObjectRequest().setBucket(bucket).setKey(key));

    svr.stop();
    serverThread.join();
    return exitCode;
}
