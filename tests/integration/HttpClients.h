#pragma once

#include <curl/curl.h>
#include <string>
#include <map>
#include <stdexcept>

namespace alibabacloud {
namespace oss2 {
namespace test {

/**
 * @brief Simple HTTP client wrapper using libcurl for integration tests.
 *
 * Provides basic GET and PUT operations for testing OSS presigned URLs.
 */
class HttpClient {
  public:
    HttpClient();
    ~HttpClient();

    /**
     * @brief Response structure containing status code, headers, and body.
     */
    struct Response {
        long statusCode = 0;
        std::string body;
        std::map<std::string, std::string> headers;
    };

    /**
     * @brief Perform a GET request to the specified URL.
     *
     * @param url The URL to request
     * @param headers Optional headers to include in the request
     * @return Response containing status code, headers, and body
     */
    Response get(const std::string& url);

    /**
     * @brief Perform a GET request to the specified URL with headers.
     *
     * @param url The URL to request
     * @param headers Headers to include in the request
     * @return Response containing status code, headers, and body
     */
    Response get(const std::string& url, const std::map<std::string, std::string>& headers);

    /**
     * @brief Perform a PUT request to the specified URL with data.
     *
     * @param url The URL to request
     * @param data The data to upload
     * @return Response containing status code, headers, and body
     */
    Response put(const std::string& url, const std::string& data);

    /**
     * @brief Perform a PUT request to the specified URL with data and headers.
     *
     * @param url The URL to request
     * @param data The data to upload
     * @param headers Headers to include in the request
     * @return Response containing status code, headers, and body
     */
    Response put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers);

    /**
     * @brief Set connection timeout in seconds.
     */
    void setConnectTimeout(long seconds);

    /**
     * @brief Set read/write timeout in seconds.
     */
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
