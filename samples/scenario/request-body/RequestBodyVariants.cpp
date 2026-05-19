// Demonstrates: Use different RequestBody factories to upload data via PutObject.
//
// The SDK provides four ways to construct a request body:
//   RequestBody::FromString  — owns a copy of the data
//   RequestBody::FromFile    — reads from a file path, re-opens on retry
//   RequestBody::FromStream  — shared_ptr<istream>, caller manages lifetime
//   RequestBody::FromMemory  — zero-copy from a raw buffer, caller keeps data alive
//
// Usage:
//   ./RequestBodyVariants --region <region> --bucket <bucket>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace oss = alibabacloud::oss2;

static bool upload(oss::OSSClient& client, const std::string& bucket,
                   const std::string& key, std::shared_ptr<oss::ByteContent> body,
                   const std::string& label) {
    auto outcome = client.putObject(
        oss::models::PutObjectRequest()
            .setBucket(bucket)
            .setKey(key)
            .setBody(body));

    if (!outcome.has_value()) {
        auto& e = outcome.error();
        std::cerr << label << " fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage() << std::endl;
        return false;
    }
    std::cout << label << " done"
              << ", status: " << outcome.value().getStatusCode()
              << ", requestId: " << outcome.value().getRequestId() << std::endl;
    return true;
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

    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = region;
    if (!endpoint.empty()) conf.endpoint = endpoint;
    conf.credentialsProvider =
        std::make_shared<oss::EnvironmentVariableCredentialsProvider>();

    auto client = oss::OSSClient(conf);
    const std::string prefix = "request-body-demo/";

    // --- 1. FromString: data is copied/moved into the body ---
    upload(client, bucket, prefix + "from-string.txt",
           oss::RequestBody::FromString("Hello from FromString!"),
           "FromString");

    // --- 2. FromFile: reads from a local file path ---
    {
        std::string tmpFile = "request-body-demo-tmp.txt";
        std::ofstream ofs(tmpFile);
        ofs << "Hello from FromFile!";
        ofs.close();

        upload(client, bucket, prefix + "from-file.txt",
               oss::RequestBody::FromFile(tmpFile),
               "FromFile");

        std::remove(tmpFile.c_str());
    }

    // --- 3. FromStream: shared_ptr<istream> ---
    {
        auto ss = std::make_shared<std::stringstream>("Hello from FromStream!");
        upload(client, bucket, prefix + "from-stream.txt",
               oss::RequestBody::FromStream(ss),
               "FromStream");
    }

    // --- 4. FromMemory: zero-copy from a raw buffer ---
    {
        const char* data = "Hello from FromMemory!";
        std::size_t len = std::strlen(data);
        upload(client, bucket, prefix + "from-memory.txt",
               oss::RequestBody::FromMemory(data, len),
               "FromMemory");
    }

    // Cleanup
    for (const auto& suffix : {"from-string.txt", "from-file.txt",
                                "from-stream.txt", "from-memory.txt"}) {
        client.deleteObject(
            oss::models::DeleteObjectRequest()
                .setBucket(bucket)
                .setKey(prefix + suffix));
    }

    std::cout << "All RequestBody variants completed successfully." << std::endl;
    return 0;
}
