
#include <alibabacloud/oss2/credentials/CredentialsProvider.h>

namespace alibabacloud {
namespace oss2 {

Credentials EnvironmentVariableCredentialsProvider::getCredentials() {
    auto ak = std::getenv("OSS_ACCESS_KEY_ID");
    auto sk = std::getenv("OSS_ACCESS_KEY_SECRET");

    if (ak == nullptr || sk == nullptr) {
        return Credentials::withError(
            "environment variables OSS_ACCESS_KEY_ID and OSS_ACCESS_KEY_SECRET must be set");
    }

    auto token = std::getenv("OSS_SESSION_TOKEN");
    if (token != nullptr) {
        return Credentials(ak, sk, token);
    }

    return Credentials(ak, sk);
}
} // namespace oss2
} // namespace alibabacloud
