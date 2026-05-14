#include "ClientHelper.h"
#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

namespace alibabacloud {
namespace oss2 {
namespace async {

std::shared_ptr<OSSAsyncClient> ClientHelper::GetDefaultClient() {
    static std::shared_ptr<OSSAsyncClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<OSSAsyncClient>(config);
    }
    return client;
}

std::shared_ptr<OSSAsyncClient> ClientHelper::GetInvalidClient() {
    static std::shared_ptr<OSSAsyncClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<StaticCredentialsProvider>("invalid-ak", "invalid-sk");
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<OSSAsyncClient>(config);
    }
    return client;
}

static void cleanBucket(OSSAsyncClient* client, const std::string& bucketName) {
    auto listOutcome = client->asyncCall(models::ListMultipartUploadsRequest().setBucket(bucketName)).get();
    if (listOutcome.isSuccess()) {
        for (auto const& upload : listOutcome.getResult().getUploads()) {
            client->asyncCall(models::AbortMultipartUploadRequest()
                    .setBucket(bucketName)
                    .setKey(upload.key)
                    .setUploadId(upload.uploadId)).get();
        }
    }

    auto request = models::ListObjectsV2Request();
    request.setBucket(bucketName);

    bool IsTruncated = false;
    do {
        auto outcome = client->asyncCall(request).get();
        if (outcome.isSuccess()) {
            for (auto const& obj : outcome.getResult().getContents()) {
                client->asyncCall(models::DeleteObjectRequest().setBucket(bucketName).setKey(obj.key)).get();
            }
            request.setContinuationToken(outcome.getResult().getNextContinuationToken());
            IsTruncated = outcome.getResult().getIsTruncated();
        } else {
            break;
        }
    } while (IsTruncated);

    client->asyncCall(models::DeleteBucketRequest().setBucket(bucketName)).get();
}

void ClientHelper::CleanBucket(const std::string& bucketName) {
    auto client = GetDefaultClient();
    cleanBucket(client.get(), bucketName);
}

void ClientHelper::CleanBucketsByPrefix(const std::string& prefix) {
    auto client = GetDefaultClient();
    auto request = models::ListBucketsRequest();
    request.setMaxKeys(1);
    request.setPrefix(prefix);
    bool IsTruncated = false;
    do {
        auto outcome = client->asyncCall(request).get();
        if (outcome.isSuccess()) {
            cleanBucket(client.get(), outcome.getResult().getBuckets()[0].name);
            request.setMarker(outcome.getResult().getNextMarker());
            IsTruncated = outcome.getResult().getIsTruncated();
        } else {
            break;
        }
    } while (IsTruncated);
}

} // namespace async
} // namespace oss2
} // namespace alibabacloud
