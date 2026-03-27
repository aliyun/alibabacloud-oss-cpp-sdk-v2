#include "alibabacloud/oss2/OSSClient.h"
#include <string>

class Config {
  public:
    static void LoadCfgFromEnv();
    static std::shared_ptr<alibabacloud::oss2::OSSClient> GetDefaultClient();
    static std::shared_ptr<alibabacloud::oss2::OSSClient> GetInvalidClient();

    static std::string GenBucketName();
    static void WaitForCacheExpire(int sec);
    static void CleanBucketsByPrefix(const std::string& prefix);
    static void CleanBucket(const std::string& bucketName);

  public:
    static std::string AccessKeyId;
    static std::string AccessKeySecret;
    static std::string Endpoint;
    static std::string Region;
    static std::string RamRoleArn;
    static std::string RamUID;
    static std::string UserID;
    static std::string AccountID;
    static std::string PayerAccessKeyId;
    static std::string PayerAccessKeySecret;
    static std::string PayerUID;
};