#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/OSSClient.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

using namespace alibabacloud::oss2;

TEST(OSSClientTest, DefaultCtor) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");
    auto client = OSSClient(config);
    client.invokeOperation(OperationInput());

    auto ptr = std::make_shared<OSSClient>(config);
}

TEST(OSSClientTest, WithClientOptionsFns) {
    auto config = ClientConfiguration::loadDefault();
    config.region = "cn-hangzhou";
    config.credentialsProvider = std::make_shared<StaticCredentialsProvider>("ak", "sk");

    ClientOptionsFns fns;
    fns.push_back([](ClientOptions& opts) {
        opts.additionalHeaders.push_back("x-custom-header");
    });

    auto client = OSSClient(config, fns);
    // Just verify it constructs successfully
    SUCCEED();
}