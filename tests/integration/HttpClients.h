#pragma once

#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_HAS_CURL

#include <curl/curl.h>
#include <string>
#include <map>
#include <stdexcept>

namespace alibabacloud {
namespace oss2 {
namespace test {

class HttpClient {
  public:
    HttpClient();
    ~HttpClient();

    struct Response {
        long statusCode = 0;
        std::string body;
        std::map<std::string, std::string> headers;
    };

    Response get(const std::string& url);
    Response get(const std::string& url, const std::map<std::string, std::string>& headers);
    Response put(const std::string& url, const std::string& data);
    Response put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers);

    void setConnectTimeout(long seconds);
    void setReadWriteTimeout(long seconds);

  private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static size_t readCallback(void* buffer, size_t size, size_t nitems, std::string* userp);
    static struct curl_slist* buildHeaderList(const std::map<std::string, std::string>& headers);

    CURL* curl_;
    long connectTimeout_ = 10;
    long readWriteTimeout_ = 30;
};

} // namespace test
} // namespace oss2
} // namespace alibabacloud

#endif // ALIBABACLOUD_OSS_HAS_CURL
