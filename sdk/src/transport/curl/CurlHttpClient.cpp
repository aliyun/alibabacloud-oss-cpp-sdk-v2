
#include "CurlHttpClient.h"
#include "CurlHelper.h"
#include "src/utils/LogUtils.h"
#include "CurlContainer.h"

#include <curl/curlver.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <charconv>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <vector>


namespace alibabacloud::oss2::transport::curl {

#define UNUSED_PARAM(x) ((void) (x))
const char* TAG = "CurlHttpClient";

namespace detail {


/////////////////////////////////////////////////////////////////////////////////////////////
struct TransferState {
    CurlHttpClient* owner;
    CURL* curl;

    RequestContext* context;
    RequestMessage* request;
    ResponseMessage* response;

    // Data from
    std::unique_ptr<ByteSource> source{};

    // Data to
    std::ostream* sink{};
    std::shared_ptr<std::ostream> userSink{};
    std::shared_ptr<std::stringstream> defaultSink{};
    bool recvFirstData{};
    int64_t recvDataLength{};
};

static size_t sendBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    TransferState* state = static_cast<TransferState*>(userdata);

    // should not happend
    if (state == nullptr || state->request == nullptr) {
        return 0;
    }

    size_t wanted = size * nmemb;
    size_t got = 0;
    if (state->source != nullptr) {
        got = state->source->readToCount(reinterpret_cast<uint8_t*>(ptr), wanted);

        // check status
        if (got < wanted) {
            auto st = state->source->state();
            if (st != 0 && (st & std::ios::eofbit) == 0) {
                return CURL_READFUNC_ABORT;
            }
        }
    }

    return got;
}

// cppcheck-suppress constParameterCallback
static size_t recvBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    TransferState* state = static_cast<TransferState*>(userdata);
    const size_t wanted = size * nmemb;

    // should not happend
    if (state == nullptr || state->response == nullptr || state->context == nullptr) {
        return 0;
    }

    if (state->recvFirstData) {
        long response_code = 0;
        curl_easy_getinfo(state->curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code / 100 != 2 || response_code == 203 || !state->context->ostreamFactory) {
            state->defaultSink = std::make_shared<std::stringstream>();
            state->sink = state->defaultSink.get();
        } else {
            state->userSink = state->context->ostreamFactory.value()(state->recvDataLength);
            state->sink = state->userSink.get();
        }
        state->recvFirstData = false;
    }

    if (state->sink == nullptr || state->sink->fail()) {
        return 0;
    }

    state->sink->write(ptr, static_cast<std::streamsize>(wanted));
    if (state->sink->bad()) {
        return 0;
    }

    return wanted;
}

static size_t recvHeaders(char* buffer, size_t size, size_t nitems, void* userdata) {
    TransferState* state = static_cast<TransferState*>(userdata);
    const size_t wanted = nitems * size;

    std::string line(buffer, size * nitems);
    auto pos = line.find(':');
    if (pos != line.npos) {
        // skip optional whitespace after ':', strip trailing \r\n
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
        state->response->headers.emplace(std::move(name), std::move(value));
    }

    if (wanted == 2 && (buffer[0] == 0x0D) && (buffer[1] == 0x0A)) {
        if (state->response->headers.find("Content-Length") != state->response->headers.end()) {
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 55, 0)
            curl_off_t dval;
            curl_easy_getinfo(state->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dval);
            state->recvDataLength = (int64_t) dval;
#else
            double dval;
            curl_easy_getinfo(state->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
            state->recvDataLength = (int64_t) dval;
#endif
        }
    }
    return wanted;
}

int debugCallback(void* handle, curl_infotype type, char* data, size_t size, void* userp) {
    UNUSED_PARAM(userp);
    switch (type) {
        default:
            break;
        case CURLINFO_TEXT:
            OSS_LOG(LogLevel::LogInfo, TAG, "handle(%p)=> Info: %.*s", handle, size, data);
            break;
        case CURLINFO_HEADER_OUT:
            OSS_LOG(LogLevel::LogDebug, TAG, "handle(%p)=> Send header: %.*s", handle, size, data);
            break;
        case CURLINFO_HEADER_IN:
            OSS_LOG(LogLevel::LogDebug, TAG, "handle(%p)=> Recv header: %.*s", handle, size, data);
            break;
    }
    return 0;
}

#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
static int xferInfoCallback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    UNUSED_PARAM(dltotal);
    UNUSED_PARAM(dlnow);
    UNUSED_PARAM(ultotal);
    UNUSED_PARAM(ulnow);
    TransferState* state = static_cast<TransferState*>(userdata);
    if (state == nullptr || state->owner == nullptr) {
        return 0;
    }

    return 0;
}
#else
// cppcheck-suppress constParameterCallback
static int progressCallback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow) {
    UNUSED_PARAM(dltotal);
    UNUSED_PARAM(dlnow);
    UNUSED_PARAM(ultotal);
    UNUSED_PARAM(ulnow);
    const TransferState* state = static_cast<const TransferState*>(userdata);
    if (state == nullptr || state->owner == nullptr) {
        return 0;
    }

    return 0;
}
#endif

bool static ignoreHeader(const std::string& header, const std::string& expect) {
    if ((expect.length() == header.length()) &&
        std::equal(header.begin(), header.end(), expect.begin(),
                   [](char a, char b) { return ::tolower(a) == ::tolower(b); })) {
        return true;
    }
    return false;
}


} // namespace detail

CurlHttpClient::CurlHttpClient(const struct HttpTransportOptions& options)
        : curlContainer_(std::make_unique<CurlContainer>(
                  16,
                  options.readWriteTimeout.value_or(10000),
                  options.connectTimeout.value_or(5000))) {
    (void)CurlHelper::instance();

    verifySSL_ = !options.insecureSkipVerify.value_or(false);

    if (options.proxyHost.has_value() && !options.proxyHost.value().empty()) {
        proxyHost_ = options.proxyHost.value();
    }
}

ResponseResult CurlHttpClient::send(std::unique_ptr<RequestMessage>& request, RequestContext& context) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter Send", request.get());

    curl_slist* list = nullptr;
    for (const auto& [k, v] : request->headers) {
        if (v.empty())
            continue;
        // ignore content-length
        if (detail::ignoreHeader(k, "Content-Length"))
            continue;
        std::string str = k;
        str.append(": ").append(v);
        list = curl_slist_append(list, str.c_str());
    }
    // Disable Expect: 100-continue
    list = curl_slist_append(list, "Expect:");

    // Add Content-Length
    // The Content-Length provided by user will be used first.
    int64_t contentlength = -1;
    if (request->headers.find("Content-Length") != request->headers.end()) {
        auto& str = request->headers.at("Content-Length");
        long long result = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
        if (ec == std::errc()) {
            contentlength = result;
        }
    }
    if (contentlength < 0 && request->body != nullptr && request->body->length().has_value()) {
        contentlength = static_cast<int64_t>(request->body->length().value());
    }
    if (contentlength >= 0) {
        std::string str = "Content-Length: ";
        str.append(std::to_string(contentlength));
        list = curl_slist_append(list, str.c_str());
    }

    auto response = std::make_unique<ResponseMessage>();

    CURL* curl = curlContainer_->Acquire();
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) acquire curl handle:%p", request.get(), curl);

    auto transferState = detail::TransferState{
            this,           /*CurlHttpClient*/
            curl,           /*CURL*/
            &context,       /*RequestContext*/
            request.get(),  /*RequestMessage*/
            response.get(), /*ResponseMessage*/
    };

    // source
    if (request->body != nullptr) {
        transferState.source = request->body->spanSource();
    }

    // sink
    transferState.recvFirstData = true;
    transferState.recvDataLength = -1;

    curl_easy_setopt(curl, CURLOPT_URL, request->uri.c_str());
    if ("HEAD" == request->method) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    } else if ("PUT" == request->method) {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        if (request->body == nullptr) {
            curl_off_t length_of_data = 0;
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, length_of_data);
        } else if (contentlength >= 0) {
            curl_off_t length_of_data = static_cast<curl_off_t>(contentlength);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, length_of_data);
        }
    } else if ("POST" == request->method) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (request->body == nullptr) {
            curl_off_t length_of_data = 0;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, length_of_data);
        } else if (contentlength >= 0) {
            curl_off_t length_of_data = static_cast<curl_off_t>(contentlength);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, length_of_data);
        }
    } else if ("DELETE" == request->method) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else {
        // default is get
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &transferState);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, detail::recvHeaders);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &transferState);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, detail::recvBody);

    curl_easy_setopt(curl, CURLOPT_READDATA, &transferState);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, detail::sendBody);

    if (verifySSL_) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    }
    if (!caPath_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, caPath_.c_str());
    }
    if (!caFile_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, caFile_.c_str());
    }

    if (!proxyHost_.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyHost_.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long) proxyPort_);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, proxyUserName_.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, proxyPassword_.c_str());
    }

    if (!networkInterface_.empty()) {
        curl_easy_setopt(curl, CURLOPT_INTERFACE, networkInterface_.c_str());
    }

    /*
    // debug
    if (GetLogLevelInner() >= LogLevel::LogInfo) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debugCallback);
    }
    */

    // Error Buffer
    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

    // Progress Callback
#if LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(7, 32, 0)
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, detail::xferInfoCallback);
#else
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, detail::progressCallback);
#endif
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &transferState);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    /*
       // Send bytes/sec
       if (sendRateLimiter_ != nullptr) {
           transferState.sendSpeed = sendRateLimiter_->Rate();
           auto speed = static_cast<curl_off_t>(transferState.sendSpeed);
           speed = speed * 1024;
           curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, speed);
       }

       // Recv bytes/sec
       if (recvRateLimiter_ != nullptr) {
           transferState.recvSpeed = recvRateLimiter_->Rate();
           auto speed = static_cast<curl_off_t>(transferState.recvSpeed);
           speed = speed * 1024;
           curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, speed);
       }

       if (httpInterceptor_ != nullptr) {
           httpInterceptor_->preSendRequest(curl, request);
       }
   */

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    } else {
        std::stringstream ss;
        ss << curl_easy_strerror(res) << "." << errbuf;
        if (res == CURLE_WRITE_ERROR) {
            if (transferState.sink == nullptr) {
                ss << ". Caused by sink is null.";
            } else if (transferState.sink->bad()) {
                ss << ". Caused by sink is in bad state(Read/writing error on i/o operation).";
            } else if (transferState.sink->fail()) {
                ss << ". Caused by sink is in fail state(Logical error on i/o operation).";
            }
        } else if (res == CURLE_ABORTED_BY_CALLBACK) {
            // TODO
        }
        context.errorCode = "CURLcode " + std::to_string(res);
        context.errorMessage = ss.str();
    }

    response->statusCode = response_code;
    response->body = transferState.defaultSink;

    curlContainer_->Release(curl, (res != CURLE_OK));

    curl_slist_free_all(list);

    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, CURLcode:%d, ResponseCode:%d", request.get(), res,
            response_code);

    return response;
}

} // namespace alibabacloud::oss2::transport::curl
