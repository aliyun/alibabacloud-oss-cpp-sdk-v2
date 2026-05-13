#pragma once

#include <curl/curl.h>

namespace alibabacloud::oss2::transport::curl {

class CurlHelper {
  public:
    static CurlHelper& instance() {
        static CurlHelper inst;
        return inst;
    }

    CurlHelper(const CurlHelper&) = delete;
    CurlHelper& operator=(const CurlHelper&) = delete;

  private:
    CurlHelper() {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlHelper() {
        curl_global_cleanup();
    }
};

} // namespace alibabacloud::oss2::transport::curl
