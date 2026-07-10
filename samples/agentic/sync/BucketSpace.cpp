// Demonstrates: Use a bucket space in two modes -- a dedicated scoped client,
// and BucketSpaceHelper combined with a regular client.
#include "SampleConfig.h"

#include "alibabacloud/oss2/agentic/OSSAgenticClient.h"
#include "alibabacloud/oss2/agentic/BucketSpaceHelper.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <sstream>

namespace oss = alibabacloud::oss2;

static oss::ClientConfiguration makeConfig(const sample::Args& args) {
    auto conf = oss::ClientConfiguration::loadDefault();
    conf.region = args.region;
    if (!args.endpoint.empty()) conf.endpoint = args.endpoint;
    conf.accountId = args.accountId;
    conf.credentialsProvider = std::make_shared<oss::EnvironmentVariableCredentialsProvider>();
    return conf;
}

int main(int argc, char* argv[]) {
    auto args = sample::parseArgs(argc, argv);
    if (args.region.empty() || args.bucket.empty() || args.key.empty() || args.accountId.empty())
        sample::printUsageAndExit(argv[0], " --account-id <accountId> --bucket <spacePrefix> --key <key>");

    auto conf = makeConfig(args);

    // ---------- Mode 1: dedicated scoped client ----------
    // makeBucketSpaceClient returns a standard OSSClient that internally resolves
    // the logical space prefix into {prefix}-{accountId}-{region}-bs-apsr.
    // Just pass the prefix; the resolver handles signing and host construction.
    auto scoped = oss::agentic::makeBucketSpaceClient(conf);

    auto putOutcome = scoped.putObject(
        oss::models::PutObjectRequest()
            .setBucket(args.bucket)  // pass the prefix only
            .setKey(args.key)
            .setBody(oss::RequestBody::fromString("hello from bucket space")));
    if (!putOutcome.has_value()) {
        auto& e = putOutcome.error();
        std::cerr << "[mode1] PutObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    std::cout << "[mode1] PutObject ok, status: " << putOutcome.value().getStatusCode()
              << ", requestId: " << putOutcome.value().getRequestId() << std::endl;

    // ---------- Mode 2: existing client + BucketSpaceHelper ----------
    // The caller manages their own client and endpoint, and resolves the physical
    // bucket name manually via BucketSpaceHelper, then passes the full name.
    oss::agentic::BucketSpaceHelper helper(conf);
    auto physicalBucket = helper.toBucketName(args.bucket);
    std::cout << "[mode2] resolved bucket name: " << physicalBucket << std::endl;

    auto client = oss::OSSClient(conf);
    auto getOutcome = client.getObject(
        oss::models::GetObjectRequest()
            .setBucket(physicalBucket)  // pass the full name
            .setKey(args.key));
    if (!getOutcome.has_value()) {
        auto& e = getOutcome.error();
        std::cerr << "[mode2] GetObject fail"
                  << ", code: " << e.getCode()
                  << ", message: " << e.getMessage()
                  << ", ec: " << e.getEC()
                  << ", requestId: " << e.getRequestId()
                  << ", requestTarget: " << e.getRequestTarget() << std::endl;
        return 1;
    }
    auto& result = getOutcome.value();
    std::cout << "[mode2] GetObject ok, status: " << result.getStatusCode()
              << ", contentLength: " << result.getContentLength() << std::endl;
    auto& body = result.getBody();
    if (body) {
        std::stringstream ss;
        ss << body->rdbuf();
        std::cout << "[mode2] content: " << ss.str() << std::endl;
    }
    return 0;
}
