#include "ClientHelper.h"
#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"

namespace alibabacloud {
namespace oss2 {
namespace sync {

std::shared_ptr<OSSClient> ClientHelper::GetDefaultClient() {
    static std::shared_ptr<OSSClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<StaticCredentialsProvider>(Config::AccessKeyId, Config::AccessKeySecret);
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<OSSClient>(config);
    }
    return client;
}

std::shared_ptr<OSSClient> ClientHelper::GetInvalidClient() {
    static std::shared_ptr<OSSClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<StaticCredentialsProvider>("invalid-ak", "invalid-sk");
        auto config = ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<OSSClient>(config);
    }
    return client;
}

static void cleanBucket(OSSClient* client, const std::string& bucketName) {
    auto listOutcome = client->listMultipartUploads(models::ListMultipartUploadsRequest().setBucket(bucketName));
    if (listOutcome.has_value()) {
        for (auto const& upload : listOutcome.value().getUploads()) {
            client->abortMultipartUpload(models::AbortMultipartUploadRequest()
                                                 .setBucket(bucketName)
                                                 .setKey(upload.key)
                                                 .setUploadId(upload.uploadId));
        }
    }

    auto request = models::ListObjectsV2Request();
    request.setBucket(bucketName);

    bool IsTruncated = false;
    do {
        auto outcome = client->listObjectsV2(request);
        if (outcome.has_value()) {
            for (auto const &obj : outcome.value().getContents()) {
                client->deleteObject(models::DeleteObjectRequest().setBucket(bucketName).setKey(obj.key));
            }
        }
        else {
            break;
        }
        request.setContinuationToken(outcome.value().getNextContinuationToken());
        IsTruncated = outcome.value().getIsTruncated();
    } while (IsTruncated);

    client->deleteBucket(models::DeleteBucketRequest().setBucket(bucketName));
}

void ClientHelper::CleanBucket(const std::string& bucketName) {
    auto client = GetDefaultClient();
    cleanBucket(client.get(), bucketName);
}

void ClientHelper::CleanBucketsByPrefix(const std::string &prefix) {
    auto client = GetDefaultClient();
    auto request = models::ListBucketsRequest();
    request.setMaxKeys(1);
    request.setPrefix(prefix);
    bool IsTruncated = false;
    do {
        auto outcome = client->listBuckets(request);
        if (outcome.has_value()) {
            cleanBucket(client.get(), outcome.value().getBuckets()[0].name);
        }
        request.setMarker(outcome.value().getNextMarker());
        IsTruncated = outcome.value().getIsTruncated();
    } while (IsTruncated);
}

} // namespace sync
} // namespace oss2
} // namespace alibabacloud
