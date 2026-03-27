#include "Config.h"
#include "alibabacloud/oss2/ClientConfiguration.h"
#include "alibabacloud/oss2/credentials/CredentialsProvider.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <thread>

std::string Config::AccessKeyId = "";
std::string Config::AccessKeySecret = "";
std::string Config::Endpoint = "";
std::string Config::Region = "";
std::string Config::RamRoleArn = "";
std::string Config::RamUID = "";
std::string Config::UserID;
std::string Config::AccountID;
std::string Config::PayerAccessKeyId = "";
std::string Config::PayerAccessKeySecret = "";
std::string Config::PayerUID = "";

static std::string LeftTrim(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](unsigned char ch) { return !::isspace(ch); }));
    return copy;
}

static std::string RightTrim(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](unsigned char ch) { return !::isspace(ch); }).base(),
               copy.end());
    return copy;
}

static std::string Trim(const char* source) {
    return LeftTrim(RightTrim(source).c_str());
}

static std::string LeftTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](int ch) { return !(ch == '"'); }));
    return copy;
}

static std::string RightTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](int ch) { return !(ch == '"'); }).base(), copy.end());
    return copy;
}

static std::string TrimQuotes(const char* source) {
    return LeftTrimQuotes(RightTrimQuotes(source).c_str());
}

void Config::LoadCfgFromEnv() {
    const char* value;
    value = std::getenv("OSS_TEST_ACCESS_KEY_ID");
    if (value) {
        Config::AccessKeyId = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ACCESS_KEY_SECRET");
    if (value) {
        Config::AccessKeySecret = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ENDPOINT");
    if (value) {
        Config::Endpoint = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_REGION");
    if (value) {
        Config::Region = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_RAM_ROLE_ARN");
    if (value) {
        Config::RamRoleArn = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_RAM_UID");
    if (value) {
        Config::RamUID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_USER_ID");
    if (value) {
        Config::UserID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ACCOUNT_ID");
    if (value) {
        Config::AccountID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_ACCESS_KEY_ID");
    if (value) {
        Config::PayerAccessKeyId = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_ACCESS_KEY_SECRET");
    if (value) {
        Config::PayerAccessKeySecret = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_UID");
    if (value) {
        Config::PayerUID = TrimQuotes(Trim(value).c_str());
    }
}

std::shared_ptr<alibabacloud::oss2::OSSClient> Config::GetDefaultClient() {
    static std::shared_ptr<alibabacloud::oss2::OSSClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<alibabacloud::oss2::StaticCredentialsProvider>(Config::AccessKeyId,
                                                                                        Config::AccessKeySecret);
        auto config = alibabacloud::oss2::ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<alibabacloud::oss2::OSSClient>(config);
    }
    return client;
}

std::shared_ptr<alibabacloud::oss2::OSSClient> Config::GetInvalidClient() {
    static std::shared_ptr<alibabacloud::oss2::OSSClient> client = nullptr;
    if (client == nullptr) {
        auto provider = std::make_shared<alibabacloud::oss2::StaticCredentialsProvider>("invalid-ak", "invalid-sk");
        auto config = alibabacloud::oss2::ClientConfiguration::loadDefault();
        config.region = Config::Region;
        config.endpoint = Config::Endpoint;
        config.credentialsProvider = provider;
        client = std::make_shared<alibabacloud::oss2::OSSClient>(config);
    }
    return client;
}

std::string Config::GenBucketName() {
    std::stringstream ss;
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    ss << "cpp-sdk-test-bucket-" << tp.time_since_epoch().count();
    return ss.str();
}

void Config::WaitForCacheExpire(int sec){
    std::this_thread::sleep_for(std::chrono::seconds(sec));
}

static void cleanBucket(alibabacloud::oss2::OSSClient* client, const std::string& bucketName) {
    using namespace alibabacloud::oss2;
    // Clean up multipart uploading object
    auto listOutcome = client->listMultipartUploads(models::ListMultipartUploadsRequest().setBucket(bucketName));
    if (listOutcome.isSuccess()) {
        for (auto const& upload : listOutcome.getResult().getUploads()) {
            client->abortMultipartUpload(models::AbortMultipartUploadRequest()
                                                 .setBucket(bucketName)
                                                 .setKey(upload.key)
                                                 .setUploadId(upload.uploadId));
        }
    }

    // Clean up objects
    auto request = models::ListObjectsV2Request();
    request.setBucket(bucketName);

    bool IsTruncated = false;
    do {
        auto outcome = client->listObjectsV2(request);
        if (outcome.isSuccess()) {
            for (auto const &obj : outcome.getResult().getContents()) {
                client->deleteObject(models::DeleteObjectRequest().setBucket(bucketName).setKey(obj.key));
            }
        }
        else {
            break;
        }
        request.setContinuationToken(outcome.getResult().getNextContinuationToken());
        IsTruncated = outcome.getResult().getIsTruncated();
    } while (IsTruncated);

    // Delete the bucket.
    client->deleteBucket(models::DeleteBucketRequest().setBucket(bucketName));
}


void Config::CleanBucket(const std::string& bucketName) {
    auto client = GetDefaultClient();
    cleanBucket(client.get(), bucketName);
}

void Config::CleanBucketsByPrefix(const std::string &prefix) {
    using namespace alibabacloud::oss2;
    auto client = GetDefaultClient();
    auto request = models::ListBucketsRequest();
    request.setMaxKeys(1);
    request.setPrefix(prefix);
    bool IsTruncated = false;
    do {
        auto outcome = client->listBuckets(request);
        if (outcome.isSuccess()) {
            cleanBucket(client.get(), outcome.getResult().getBuckets()[0].name);
        }
        request.setMarker(outcome.getResult().getNextMarker());
        IsTruncated = outcome.getResult().getIsTruncated();
    } while (IsTruncated);
}
