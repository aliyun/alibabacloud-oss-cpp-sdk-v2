// Demonstrates: Multipart upload using presigned URLs.
//
// Interaction flow:
//
//   Client                     App Server (your backend)           OSS
//     |                               |                              |
//     |  1. request upload (fileSize) |                              |
//     |------------------------------>|                              |
//     |                               |  2. InitiateMultipartUpload  |
//     |                               |----------------------------->|
//     |                               |         uploadId             |
//     |                               |<-----------------------------|
//     |                               |  3. presign(UploadPart) x N  |
//     |                               |  (local signing, no network) |
//     |  4. return uploadId + URLs    |                              |
//     |<------------------------------|                              |
//     |                                                              |
//     |  5. HTTP PUT part 1 (sequential or parallel)                 |
//     |------------------------------------------------------------->|
//     |                                          200 OK + ETag       |
//     |<-------------------------------------------------------------|
//     |  ...repeat for each part (can upload concurrently)...        |
//     |                                                              |
//     |  6. report ETags              |                              |
//     |------------------------------>|                              |
//     |                               |  7. CompleteMultipartUpload  |
//     |                               |----------------------------->|
//     |                               |         200 OK               |
//     |                               |<-----------------------------|
//     |  8. upload complete           |                              |
//     |<------------------------------|                              |
//
// This sample starts an App Server (httplib) and a Client in the same process
// to demonstrate the full interaction.  In production they are separate apps.
//
// Usage:
//   ./PresignMultipartUpload --region <region> --bucket <bucket>
//                            [--endpoint <endpoint>]

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <httplib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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

    // App Server route: initiate multipart upload and return presigned part URLs
    //   POST /multipart/init?key=xxx&parts=N
    //   Response body (line-based):
    //     Line 1: uploadId
    //     Line 2+: for each part, a block of:
    //       --part <partNumber>
    //       url:<presigned URL>
    //       header lines as name:value
    svr.Post("/multipart/init", [&](const httplib::Request& req, httplib::Response& res) {
        auto key = req.get_param_value("key");
        auto partsStr = req.get_param_value("parts");
        if (key.empty() || partsStr.empty()) {
            res.status = 400;
            res.set_content("missing 'key' or 'parts' parameter", "text/plain");
            return;
        }
        int partCount = std::stoi(partsStr);

        auto initOutcome = ossClient.initiateMultipartUpload(
            oss::models::InitiateMultipartUploadRequest()
                .setBucket(bucket)
                .setKey(key));
        if (!initOutcome.has_value()) {
            res.status = 500;
            res.set_content("InitiateMultipartUpload failed: " +
                initOutcome.error().getMessage(), "text/plain");
            return;
        }

        std::string uploadId = initOutcome.value().getUploadId();

        oss::models::PresignOptions presignOpts;
        presignOpts.setExpirationDuration(std::chrono::seconds(3600));

        std::string body = uploadId + "\n";
        for (int i = 1; i <= partCount; i++) {
            auto presignOutcome = ossClient.presign(
                oss::models::UploadPartRequest()
                    .setBucket(bucket)
                    .setKey(key)
                    .setUploadId(uploadId)
                    .setPartNumber(i),
                &presignOpts);
            if (!presignOutcome.has_value()) {
                ossClient.abortMultipartUpload(
                    oss::models::AbortMultipartUploadRequest()
                        .setBucket(bucket)
                        .setKey(key)
                        .setUploadId(uploadId));
                res.status = 500;
                res.set_content("presign part " + std::to_string(i) + " failed", "text/plain");
                return;
            }
            auto& r = presignOutcome.value();
            body += "--part " + std::to_string(i) + "\n";
            body += "url:" + r.getUrl() + "\n";
            for (const auto& h : r.getSignedHeaders()) {
                body += h.first + ":" + h.second + "\n";
            }
        }
        res.set_content(body, "text/plain");
    });

    // App Server route: complete multipart upload
    //   POST /multipart/complete?key=xxx&uploadId=xxx
    //   Request body (line-based):
    //     partNumber:ETag (one per line)
    //   Response: 200 OK or error
    svr.Post("/multipart/complete", [&](const httplib::Request& req, httplib::Response& res) {
        auto key = req.get_param_value("key");
        auto uploadId = req.get_param_value("uploadId");
        if (key.empty() || uploadId.empty()) {
            res.status = 400;
            res.set_content("missing 'key' or 'uploadId' parameter", "text/plain");
            return;
        }

        std::vector<oss::models::Part> parts;
        std::istringstream iss(req.body);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                int partNumber = std::stoi(line.substr(0, colon));
                std::string etag = line.substr(colon + 1);
                parts.push_back(
                    oss::models::Part().setPartNumber(partNumber).setETag(etag));
            }
        }

        auto completeOutcome = ossClient.completeMultipartUpload(
            oss::models::CompleteMultipartUploadRequest()
                .setBucket(bucket)
                .setKey(key)
                .setUploadId(uploadId)
                .setCompleteMultipartUpload(
                    oss::models::CompleteMultipartUpload().setParts(parts)));
        if (!completeOutcome.has_value()) {
            res.status = 500;
            res.set_content("CompleteMultipartUpload failed: " +
                completeOutcome.error().getMessage(), "text/plain");
            return;
        }

        auto& result = completeOutcome.value();
        res.set_content("ETag:" + result.getETag() + "\n"
                        "Location:" + result.getLocation() + "\n", "text/plain");
    });

    int port = svr.bind_to_any_port("127.0.0.1");
    std::thread serverThread([&svr]() { svr.listen_after_bind(); });
    svr.wait_until_ready();
    std::cout << "[App Server] listening on http://127.0.0.1:" << port << std::endl;

    // ===== Client =====

    int exitCode = 0;
    std::string key = "presign-demo/multipart-sample.dat";

    // Simulate a file split into 3 parts (100 KB minimum per part in real usage)
    const std::size_t partSize = 100 * 1024;  // 100 KB
    const int partCount = 3;
    std::string fileData(partSize * partCount, '\0');
    for (std::size_t i = 0; i < fileData.size(); i++) {
        fileData[i] = static_cast<char>('A' + (i % 26));
    }

    // Step 1: request multipart init from App Server
    std::cout << "\n[Client] requesting multipart upload init..." << std::endl;
    httplib::Client appClient("http://127.0.0.1:" + std::to_string(port));
    auto initRes = appClient.Post("/multipart/init?key=" + key +
                                  "&parts=" + std::to_string(partCount));
    if (!initRes || initRes->status != 200) {
        std::cerr << "[Client] failed to init multipart upload" << std::endl;
        svr.stop();
        serverThread.join();
        return 1;
    }

    // Parse response: first line is uploadId, then --part blocks
    std::istringstream initStream(initRes->body);
    std::string uploadId;
    std::getline(initStream, uploadId);
    std::cout << "[Client] uploadId: " << uploadId << std::endl;

    struct PartInfo {
        int partNumber;
        std::string url;
        httplib::Headers headers;
    };
    std::vector<PartInfo> partInfos;

    std::string line;
    PartInfo current{};
    while (std::getline(initStream, line)) {
        if (line.empty()) continue;
        if (line.rfind("--part ", 0) == 0) {
            if (current.partNumber > 0) partInfos.push_back(current);
            current = {};
            current.partNumber = std::stoi(line.substr(7));
        } else if (line.rfind("url:", 0) == 0) {
            current.url = line.substr(4);
        } else {
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                current.headers.emplace(line.substr(0, colon), line.substr(colon + 1));
            }
        }
    }
    if (current.partNumber > 0) partInfos.push_back(current);

    // Step 2: upload each part via HTTP PUT to OSS
    std::cout << "\n[Client] uploading " << partInfos.size() << " parts to OSS..." << std::endl;
    std::string completeBody;

    for (auto& pi : partInfos) {
        std::size_t offset = static_cast<std::size_t>(pi.partNumber - 1) * partSize;
        std::string partData = fileData.substr(offset, partSize);

        auto [base, path] = splitUrl(pi.url);
        httplib::Client ossHttp(base);
        ossHttp.set_connection_timeout(std::chrono::seconds(10));
        ossHttp.set_read_timeout(std::chrono::seconds(60));
        ossHttp.set_follow_location(true);
        ossHttp.set_url_encode(false);

        auto putRes = ossHttp.Put(path, pi.headers, partData, "application/octet-stream");
        if (!putRes || putRes->status != 200) {
            std::cerr << "[Client] upload part " << pi.partNumber << " failed" << std::endl;
            exitCode = 1;
            break;
        }

        std::string etag = putRes->get_header_value("ETag");
        completeBody += std::to_string(pi.partNumber) + ":" + etag + "\n";
        std::cout << "  Part " << pi.partNumber << " uploaded, ETag: " << etag << std::endl;
    }

    // Step 3: report ETags to App Server to complete multipart upload
    if (exitCode == 0) {
        std::cout << "\n[Client] completing multipart upload..." << std::endl;
        auto completeRes = appClient.Post(
            "/multipart/complete?key=" + key + "&uploadId=" + uploadId,
            completeBody, "text/plain");
        if (completeRes && completeRes->status == 200) {
            std::cout << "[Client] multipart upload completed!\n"
                      << completeRes->body << std::endl;
        } else {
            std::cerr << "[Client] complete multipart upload failed" << std::endl;
            exitCode = 1;
        }
    }

    // Cleanup
    ossClient.deleteObject(
        oss::models::DeleteObjectRequest().setBucket(bucket).setKey(key));

    svr.stop();
    serverThread.join();
    return exitCode;
}
