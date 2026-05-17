#include "HttpClients.h"

#ifdef ALIBABACLOUD_OSS_HAS_CURL

#include <cstring>
#include <cassert>

namespace alibabacloud {
namespace oss2 {
namespace test {

HttpClient::HttpClient() : curl_(curl_easy_init()) {
    if (!curl_) {
        assert(false && "Failed to initialize libcurl");
    }
}

HttpClient::~HttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t HttpClient::readCallback(void* buffer, size_t size, size_t nitems, std::string* userp) {
    size_t totalSize = size * nitems;
    size_t availableSize = userp->size();
    if (availableSize > totalSize) {
        std::memcpy(buffer, userp->data(), totalSize);
        *userp = userp->substr(totalSize);
        return totalSize;
    } else {
        std::memcpy(buffer, userp->data(), availableSize);
        userp->clear();
        return availableSize;
    }
}

void HttpClient::setConnectTimeout(long seconds) {
    connectTimeout_ = seconds;
}

void HttpClient::setReadWriteTimeout(long seconds) {
    readWriteTimeout_ = seconds;
}

struct curl_slist* HttpClient::buildHeaderList(const std::map<std::string, std::string>& headers) {
    struct curl_slist* headerList = nullptr;
    for (const auto& [key, value] : headers) {
        std::string headerLine = key + ": " + value;
        headerList = curl_slist_append(headerList, headerLine.c_str());
    }
    return headerList;
}

HttpClient::Response HttpClient::get(const std::string& url) {
    return get(url, std::map<std::string, std::string>{});
}

HttpClient::Response HttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers) {
    Response response;

    if (!curl_) {
        response.statusCode = 0;
        return response;
    }

    curl_easy_reset(curl_);

    // Set URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    // Set timeouts
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, connectTimeout_);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, readWriteTimeout_);

    // Follow redirects
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 5L);

    // Set headers
    struct curl_slist* headerList = buildHeaderList(headers);
    if (headerList) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headerList);
    }

    // Set write callback for response body
    std::string responseBody;
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &responseBody);

    // Perform the request
    CURLcode res = curl_easy_perform(curl_);

    // Get response code
    long responseCode = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode);
    } else {
        responseCode = 0;
    }

    // Clean up headers
    if (headerList) {
        curl_slist_free_all(headerList);
    }

    response.statusCode = responseCode;
    response.body = responseBody;

    return response;
}

HttpClient::Response HttpClient::put(const std::string& url, const std::string& data) {
    return put(url, data, std::map<std::string, std::string>{});
}

HttpClient::Response HttpClient::put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
    Response response;

    if (!curl_) {
        response.statusCode = 0;
        return response;
    }

    // Create a mutable copy of data for the read callback
    std::string requestData = data;

    curl_easy_reset(curl_);

    // Set URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    // Set timeouts
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, connectTimeout_);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, readWriteTimeout_);

    // Follow redirects
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 5L);

    // Set PUT method using UPLOAD flag
    curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);

    // Set request body size
    curl_easy_setopt(curl_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(data.size()));

    // Set read callback for request body
    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl_, CURLOPT_READDATA, &requestData);

    // Set headers
    struct curl_slist* headerList = buildHeaderList(headers);
    if (headerList) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headerList);
    }

    // Set write callback for response body
    std::string responseBody;
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &responseBody);

    // Perform the request
    CURLcode res = curl_easy_perform(curl_);

    // Get response code
    long responseCode = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode);
    } else {
        responseCode = 0;
    }

    // Clean up headers
    if (headerList) {
        curl_slist_free_all(headerList);
    }

    response.statusCode = responseCode;
    response.body = responseBody;

    return response;
}

} // namespace test
} // namespace oss2
} // namespace alibabacloud

#endif // ALIBABACLOUD_OSS_HAS_CURL
