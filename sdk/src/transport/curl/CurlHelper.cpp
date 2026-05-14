#include "CurlHelper.h"

#include <curl/curlver.h>

#include <algorithm>
#include <charconv>

namespace alibabacloud::oss2::transport::curl {

// cppcheck-suppress constParameterPointer
size_t sendBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* io = static_cast<TransferIO*>(userdata);
    if (io == nullptr || io->request == nullptr) {
        return 0;
    }

    size_t wanted = size * nmemb;
    size_t got = 0;
    if (io->source != nullptr) {
        got = io->source->readToCount(reinterpret_cast<uint8_t*>(ptr), wanted);
        if (got < wanted) {
            auto st = io->source->state();
            if (st != 0 && (st & std::ios::eofbit) == 0) {
                return CURL_READFUNC_ABORT;
            }
        }
    }
    return got;
}

// cppcheck-suppress constParameterPointer
size_t recvBodyCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* io = static_cast<TransferIO*>(userdata);
    const size_t wanted = size * nmemb;

    if (io == nullptr || io->response == nullptr) {
        return 0;
    }

    if (io->recvFirstData) {
        long response_code = 0;
        curl_easy_getinfo(io->curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code / 100 != 2 || response_code == 203 ||
            io->ostreamFactory == nullptr || !io->ostreamFactory->has_value()) {
            io->defaultSink = std::make_shared<std::stringstream>();
            io->sink = io->defaultSink.get();
        } else {
            io->userSink = io->ostreamFactory->value()(io->recvDataLength);
            io->sink = io->userSink.get();
        }
        io->recvFirstData = false;
    }

    if (io->sink == nullptr || io->sink->fail()) {
        return 0;
    }

    io->sink->write(ptr, static_cast<std::streamsize>(wanted));
    if (io->sink->bad()) {
        return 0;
    }
    return wanted;
}

// cppcheck-suppress constParameterPointer
size_t recvHeadersCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* io = static_cast<TransferIO*>(userdata);
    const size_t wanted = nitems * size;

    std::string line(buffer, wanted);
    auto pos = line.find(':');
    if (pos != line.npos) {
        size_t valueStart = pos + 1;
        while (valueStart < line.size() && (line[valueStart] == ' ' || line[valueStart] == '\t')) {
            ++valueStart;
        }
        size_t valueEnd = line.size();
        while (valueEnd > valueStart && (line[valueEnd - 1] == '\r' || line[valueEnd - 1] == '\n')) {
            --valueEnd;
        }
        auto name = line.substr(0, pos);
        auto value = line.substr(valueStart, valueEnd - valueStart);
        io->response->headers.emplace(std::move(name), std::move(value));
    }

    if (wanted == 2 && (buffer[0] == 0x0D) && (buffer[1] == 0x0A)) {
        if (io->response->headers.find("Content-Length") != io->response->headers.end()) {
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 55, 0)
            curl_off_t dval;
            curl_easy_getinfo(io->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dval);
            io->recvDataLength = (int64_t) dval;
#else
            double dval;
            curl_easy_getinfo(io->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
            io->recvDataLength = (int64_t) dval;
#endif
        }
    }
    return wanted;
}

bool headerNameEquals(const std::string& header, const std::string& expect) {
    return (expect.length() == header.length()) &&
           std::equal(header.begin(), header.end(), expect.begin(),
                      [](char a, char b) { return ::tolower(a) == ::tolower(b); });
}

curl_slist* buildHeaderList(const HeaderCollection& headers,
                            const std::shared_ptr<ByteContent>& body,
                            int64_t& contentLength) {
    curl_slist* list = nullptr;
    for (const auto& [k, v] : headers) {
        if (v.empty()) continue;
        if (headerNameEquals(k, "Content-Length")) continue;
        std::string str = k;
        str.append(": ").append(v);
        list = curl_slist_append(list, str.c_str());
    }
    list = curl_slist_append(list, "Expect:");

    contentLength = -1;
    if (headers.find("Content-Length") != headers.end()) {
        auto& str = headers.at("Content-Length");
        long long result = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
        if (ec == std::errc()) {
            contentLength = result;
        }
    }
    if (contentLength < 0 && body != nullptr && body->length().has_value()) {
        contentLength = static_cast<int64_t>(body->length().value());
    }
    if (contentLength >= 0) {
        std::string str = "Content-Length: ";
        str.append(std::to_string(contentLength));
        list = curl_slist_append(list, str.c_str());
    }

    return list;
}

void applyHttpMethod(CURL* curl, const std::string& method,
                     const std::shared_ptr<ByteContent>& body,
                     int64_t contentLength) {
    if ("HEAD" == method) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    } else if ("PUT" == method) {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        if (body == nullptr) {
            curl_off_t len = 0;
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);
        } else if (contentLength >= 0) {
            curl_off_t len = static_cast<curl_off_t>(contentLength);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, len);
        }
    } else if ("POST" == method) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body == nullptr) {
            curl_off_t len = 0;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, len);
        } else if (contentLength >= 0) {
            curl_off_t len = static_cast<curl_off_t>(contentLength);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, len);
        }
    } else if ("DELETE" == method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
}

void applyConnectionOptions(CURL* curl, const ConnectionOptions& opts,
                            const RequestMessage* request) {
    if (opts.verifySSL) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (!opts.caPath.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, opts.caPath.c_str());
    }
    if (!opts.caFile.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, opts.caFile.c_str());
    }

    if (!opts.proxyHost.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, opts.proxyHost.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long) opts.proxyPort);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, opts.proxyUserName.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, opts.proxyPassword.c_str());
    }

    if (!opts.networkInterface.empty()) {
        curl_easy_setopt(curl, CURLOPT_INTERFACE, opts.networkInterface.c_str());
    }

    if (opts.enabledRedirect) {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    }

    if (opts.enableVerbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    if (opts.requestInterceptor) {
        opts.requestInterceptor(curl, request);
    }
}

ConnectionOptions buildConnectionOptions(const HttpTransportOptions& options) {
    ConnectionOptions opts;
    opts.verifySSL = !options.insecureSkipVerify.value_or(false);
    opts.enabledRedirect = options.enabledRedirect.value_or(false);
    if (options.proxyHost.has_value() && !options.proxyHost.value().empty()) {
        opts.proxyHost = options.proxyHost.value();
    }
    return opts;
}

ConnectionOptions buildConnectionOptions(const CurlTransportOptions& options) {
    ConnectionOptions opts = buildConnectionOptions(static_cast<const HttpTransportOptions&>(options));
    if (options.proxyPort.has_value()) {
        opts.proxyPort = options.proxyPort.value();
    }
    if (options.proxyUserName.has_value()) {
        opts.proxyUserName = options.proxyUserName.value();
    }
    if (options.proxyPassword.has_value()) {
        opts.proxyPassword = options.proxyPassword.value();
    }
    if (options.caPath.has_value()) {
        opts.caPath = options.caPath.value();
    }
    if (options.caFile.has_value()) {
        opts.caFile = options.caFile.value();
    }
    if (options.networkInterface.has_value()) {
        opts.networkInterface = options.networkInterface.value();
    }
    if (options.enableVerbose.has_value()) {
        opts.enableVerbose = options.enableVerbose.value();
    }
    if (options.requestInterceptor) {
        opts.requestInterceptor = options.requestInterceptor;
    }
    return opts;
}

} // namespace alibabacloud::oss2::transport::curl
