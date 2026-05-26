#include "CurlHelper.h"
#include "alibabacloud/oss2/Error.h"

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
            io->sinkFactory == nullptr || !io->sinkFactory->has_value()) {
            io->defaultSink = std::make_shared<std::stringstream>();
            io->userSink = std::make_shared<OStreamWriter>(io->defaultSink);
            io->sink = io->userSink.get();
        } else {
            io->userSink = io->sinkFactory->value()(io->recvDataLength, io->response->headers);
            io->sink = io->userSink.get();
        }
        io->recvFirstData = false;
    }

    if (io->sink == nullptr || io->sink->fail()) {
        return 0;
    }

    io->sink->write(reinterpret_cast<const std::uint8_t*>(ptr), wanted);
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

void applyClientOptions(CURL* curl, const ClientOptions& opts,
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

ClientOptions buildClientOptions(const HttpTransportOptions& options) {
    ClientOptions opts;
    opts.verifySSL = !options.insecureSkipVerify.value_or(false);
    opts.enabledRedirect = options.enabledRedirect.value_or(false);
    if (options.proxyHost.has_value() && !options.proxyHost.value().empty()) {
        opts.proxyHost = options.proxyHost.value();
    }
    if (options.isRequestDisabled) {
        opts.isRequestDisabled = options.isRequestDisabled;
    }
    return opts;
}

ClientOptions buildClientOptions(const CurlTransportOptions& options) {
    ClientOptions opts = buildClientOptions(static_cast<const HttpTransportOptions&>(options));
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

std::string curlVersionString() {
    auto* info = curl_version_info(CURLVERSION_NOW);
    if (info == nullptr || info->version == nullptr) {
        return "unknown";
    }
    return info->version;
}

static TransportErrorCode curlCodeToTransportError(int curlCode) {
    switch (curlCode) {
        case 5:  return TransportErrorCode::DnsError;          // CURLE_COULDNT_RESOLVE_PROXY
        case 6:  return TransportErrorCode::DnsError;          // CURLE_COULDNT_RESOLVE_HOST
        case 7:  return TransportErrorCode::ConnectionFailed;  // CURLE_COULDNT_CONNECT
        case 18: return TransportErrorCode::PartialTransfer;   // CURLE_PARTIAL_FILE
        case 23: return TransportErrorCode::SendRecvError;     // CURLE_WRITE_ERROR
        case 28: return TransportErrorCode::Timeout;           // CURLE_OPERATION_TIMEDOUT
        case 35: return TransportErrorCode::ConnectionFailed;  // CURLE_SSL_CONNECT_ERROR
        case 51: return TransportErrorCode::SslError;          // CURLE_PEER_FAILED_VERIFICATION
        case 53: return TransportErrorCode::SslError;          // CURLE_SSL_ENGINE_NOTFOUND
        case 55: return TransportErrorCode::SendRecvError;     // CURLE_SEND_ERROR
        case 56: return TransportErrorCode::SendRecvError;     // CURLE_RECV_ERROR
        case 52: return TransportErrorCode::SendRecvError;     // CURLE_GOT_NOTHING
        case 65: return TransportErrorCode::SendRecvError;     // CURLE_SEND_FAIL_REWIND
        default: return TransportErrorCode::Unknown;
    }
}

std::error_code make_transport_error_code(int curlCode) {
    return make_error_code(curlCodeToTransportError(curlCode));
}

static bool shouldAbortTransfer(const TransferIO* io) {
    if (io->cancellationToken != nullptr && io->cancellationToken->isCanceled()) {
        return true;
    }
    if (io->isRequestDisabled != nullptr && (*io->isRequestDisabled)()) {
        return true;
    }
    return false;
}

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
static int xferInfoCallback(void* userdata, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    const auto* io = static_cast<const TransferIO*>(userdata);
    if (io == nullptr) {
        return 0;
    }
    if (shouldAbortTransfer(io)) {
        return 1;
    }
    return 0;
}
#else
static int progressCallback(void* userdata, double, double, double, double) {
    const auto* io = static_cast<const TransferIO*>(userdata);
    if (io == nullptr) {
        return 0;
    }
    if (shouldAbortTransfer(io)) {
        return 1;
    }
    return 0;
}
#endif

TransportError buildTransportError(CURLcode res, const char* errbuf,
                                    const TransferIO& io) {
    if (res == CURLE_ABORTED_BY_CALLBACK && shouldAbortTransfer(&io)) {
        return TransportError{make_error_code(TransportErrorCode::Canceled),
                              "RequestCanceled", "Request canceled by CancellationToken"};
    }

    std::stringstream ss;
    ss << curl_easy_strerror(res) << "." << errbuf;
    if (res == CURLE_WRITE_ERROR) {
        if (io.sink == nullptr) {
            ss << ". Caused by sink is null.";
        } else if (io.sink->bad()) {
            ss << ". Caused by sink is in bad state(Read/writing error on i/o operation).";
        } else if (io.sink->fail()) {
            ss << ". Caused by sink is in fail state(Logical error on i/o operation).";
        }
    }

    return TransportError{make_transport_error_code(res),
                          "CURLcode " + std::to_string(res),
                          ss.str()};
}

void applyRequestOptions(CURL* curl, TransferIO* io) {
    if (io->cancellationToken != nullptr || io->isRequestDisabled != nullptr) {
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferInfoCallback);
#else
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
#endif
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, io);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }
}

} // namespace alibabacloud::oss2::transport::curl
