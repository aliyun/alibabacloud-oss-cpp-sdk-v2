
#include "WinHttpClient.h"
#include "src/utils/LogUtils.h"

#include <charconv>
#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace transport {
namespace winhttp {

static const char* TAG = "WinHttpClient";
static const uint32_t HTTP_WRITE_BUFFER_LENGTH = 8192;

std::wstring WinHttpClient::ToWideString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstr(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
    return wstr;
}

std::string WinHttpClient::FromWideString(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string str(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], size, nullptr, nullptr);
    return str;
}

WinHttpClient::WinHttpClient(const HttpTransportOptions& options)
    : hSession_(nullptr),
      verifySSL_(true),
      enabledRedirect_(true) {

    if (options.insecureSkipVerify.has_value() && options.insecureSkipVerify.value()) {
        verifySSL_ = false;
    }

    if (options.enabledRedirect.has_value()) {
        enabledRedirect_ = options.enabledRedirect.value();
    }

    if (options.proxyHost.has_value()) {
        proxyHost_ = options.proxyHost.value();
    }

    long connectTimeout = options.connectTimeout.value_or(5000);
    long requestTimeout = options.readWriteTimeout.value_or(10000);

    DWORD accessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
    LPCWSTR proxyName = WINHTTP_NO_PROXY_NAME;
    std::wstring wProxyHost;

    if (!proxyHost_.empty()) {
        accessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        wProxyHost = ToWideString(proxyHost_);
        proxyName = wProxyHost.c_str();
    }

    hSession_ = WinHttpOpen(
        L"alibabacloud-oss-cpp-sdk-v2",
        accessType,
        proxyName,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (hSession_ == nullptr) {
        OSS_LOG(LogLevel::LogError, TAG, "Failed to open WinHttp session, error: %lu", GetLastError());
        return;
    }

    WinHttpSetTimeouts(hSession_, connectTimeout, connectTimeout, requestTimeout, requestTimeout);

    if (verifySSL_) {
        DWORD flags = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 |
                      WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 |
                      WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(hSession_, WINHTTP_OPTION_SECURE_PROTOCOLS, &flags, sizeof(flags));
    }

    connectionPool_ = std::make_unique<WinHttpConnectionPool>(
        hSession_, 16, connectTimeout, requestTimeout);
}

WinHttpClient::~WinHttpClient() {
    connectionPool_.reset();
    if (hSession_ != nullptr) {
        WinHttpCloseHandle(hSession_);
        hSession_ = nullptr;
    }
}

ResponseResult WinHttpClient::send(std::unique_ptr<RequestMessage>& request, RequestContext& context) {
    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) enter Send", request.get());

    auto response = std::make_unique<ResponseMessage>();

    // Parse URL
    URL_COMPONENTS urlComp;
    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength = static_cast<DWORD>(-1);
    urlComp.dwHostNameLength = static_cast<DWORD>(-1);
    urlComp.dwUrlPathLength = static_cast<DWORD>(-1);
    urlComp.dwExtraInfoLength = static_cast<DWORD>(-1);

    std::wstring wUrl = ToWideString(request->uri);
    if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.length()), 0, &urlComp)) {
        context.errorCode = "WinHttpError";
        context.errorMessage = "Failed to parse URL: " + request->uri;
        return response;
    }

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    uint16_t port = urlComp.nPort;
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
    if (urlComp.dwExtraInfoLength > 0) {
        path.append(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
    }

    bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    // Acquire connection from pool
    HINTERNET hConnect = connectionPool_->AcquireConnection(FromWideString(host), port);
    if (hConnect == nullptr) {
        context.errorCode = "WinHttpError";
        context.errorMessage = "Failed to connect to host";
        return response;
    }

    // Open request
    DWORD requestFlags = 0;
    if (isHttps) {
        requestFlags |= WINHTTP_FLAG_SECURE;
    }

    std::wstring wMethod = ToWideString(request->method);
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        wMethod.c_str(),
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        requestFlags);

    if (hRequest == nullptr) {
        context.errorCode = "WinHttpError";
        context.errorMessage = "Failed to open request";
        connectionPool_->ReleaseConnection(FromWideString(host), port, hConnect);
        return response;
    }

    // SSL verification
    if (!verifySSL_) {
        DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                         SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));
    }

    // Disable redirects if configured
    if (!enabledRedirect_) {
        DWORD disableFlags = WINHTTP_DISABLE_REDIRECTS;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disableFlags, sizeof(disableFlags));
    }

    // Add request headers
    // Calculate Content-Length
    int64_t contentLength = -1;
    if (request->headers.find("Content-Length") != request->headers.end()) {
        auto& str = request->headers.at("Content-Length");
        long long result = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
        if (ec == std::errc()) {
            contentLength = result;
        }
    }
    if (contentLength < 0 && request->body != nullptr && request->body->length().has_value()) {
        contentLength = static_cast<int64_t>(request->body->length().value());
    }

    std::wstring headerStr;
    for (const auto& [k, v] : request->headers) {
        if (v.empty()) continue;
        headerStr.append(ToWideString(k));
        headerStr.append(L": ");
        headerStr.append(ToWideString(v));
        headerStr.append(L"\r\n");
    }

    if (!headerStr.empty()) {
        WinHttpAddRequestHeaders(hRequest, headerStr.c_str(),
            static_cast<DWORD>(headerStr.length()),
            WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD);
    }

    // Send request
    DWORD totalLength = (contentLength >= 0) ? static_cast<DWORD>(contentLength) : 0;
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, totalLength, 0)) {
        DWORD err = GetLastError();
        context.errorCode = "WinHttpError " + std::to_string(err);
        context.errorMessage = "Failed to send request";
        WinHttpCloseHandle(hRequest);
        connectionPool_->ReleaseConnection(FromWideString(host), port, hConnect);
        return response;
    }

    // Write request body
    if (request->body != nullptr && contentLength > 0) {
        auto source = request->body->spanSource();
        if (source != nullptr) {
            char buffer[HTTP_WRITE_BUFFER_LENGTH];
            bool writeSuccess = true;
            while (writeSuccess) {
                size_t got = source->readToCount(reinterpret_cast<uint8_t*>(buffer), HTTP_WRITE_BUFFER_LENGTH);
                if (got == 0) break;

                DWORD bytesWritten = 0;
                writeSuccess = WinHttpWriteData(hRequest, buffer, static_cast<DWORD>(got), &bytesWritten) != 0;
                if (!writeSuccess) {
                    DWORD err = GetLastError();
                    context.errorCode = "WinHttpError " + std::to_string(err);
                    context.errorMessage = "Failed to write request body";
                    WinHttpCloseHandle(hRequest);
                    connectionPool_->ReleaseConnection(FromWideString(host), port, hConnect);
                    return response;
                }
            }
        }
    }

    // Receive response
    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        DWORD err = GetLastError();
        context.errorCode = "WinHttpError " + std::to_string(err);
        context.errorMessage = "Failed to receive response";
        WinHttpCloseHandle(hRequest);
        connectionPool_->ReleaseConnection(FromWideString(host), port, hConnect);
        return response;
    }

    // Read status code
    {
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
        response->statusCode = static_cast<long>(statusCode);
    }

    // Read response headers
    {
        DWORD headerSize = 0;
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_RAW_HEADERS_CRLF,
            WINHTTP_HEADER_NAME_BY_INDEX,
            nullptr, &headerSize, WINHTTP_NO_HEADER_INDEX);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && headerSize > 0) {
            std::vector<wchar_t> headerBuf(headerSize / sizeof(wchar_t));
            if (WinHttpQueryHeaders(hRequest,
                    WINHTTP_QUERY_RAW_HEADERS_CRLF,
                    WINHTTP_HEADER_NAME_BY_INDEX,
                    headerBuf.data(), &headerSize, WINHTTP_NO_HEADER_INDEX)) {

                std::string headers = FromWideString(std::wstring(headerBuf.data()));
                std::istringstream stream(headers);
                std::string line;
                while (std::getline(stream, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    auto pos = line.find(':');
                    if (pos != std::string::npos && pos + 2 <= line.size()) {
                        auto name = line.substr(0, pos);
                        auto value = line.substr(pos + 2);
                        // Trim trailing whitespace
                        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
                            value.pop_back();
                        }
                        response->headers.emplace(std::move(name), std::move(value));
                    }
                }
            }
        }
    }

    // Read response body
    int64_t recvDataLength = -1;
    if (response->headers.find("Content-Length") != response->headers.end()) {
        auto& cl = response->headers.at("Content-Length");
        long long val = 0;
        auto [ptr, ec] = std::from_chars(cl.data(), cl.data() + cl.size(), val);
        if (ec == std::errc()) {
            recvDataLength = val;
        }
    }

    {
        std::shared_ptr<std::ostream> sink;
        bool isError = (response->statusCode / 100 != 2) || (response->statusCode == 203);
        auto defaultSink = std::make_shared<std::stringstream>();

        if (isError || !context.ostreamFactory) {
            sink = defaultSink;
        } else {
            sink = context.ostreamFactory.value()(recvDataLength);
        }

        DWORD available = 0;
        while (WinHttpQueryDataAvailable(hRequest, &available) && available > 0) {
            std::vector<char> buf(available);
            DWORD bytesRead = 0;
            if (WinHttpReadData(hRequest, buf.data(), available, &bytesRead) && bytesRead > 0) {
                sink->write(buf.data(), static_cast<std::streamsize>(bytesRead));
                if (sink->bad()) {
                    break;
                }
            } else {
                break;
            }
            available = 0;
        }

        if (isError || !context.ostreamFactory) {
            response->body = defaultSink;
        }
    }

    WinHttpCloseHandle(hRequest);
    connectionPool_->ReleaseConnection(FromWideString(host), port, hConnect);

    OSS_LOG(LogLevel::LogDebug, TAG, "request(%p) leave Send, ResponseCode:%ld",
            request.get(), response->statusCode);

    return response;
}

} // namespace winhttp
} // namespace transport
} // namespace oss2
} // namespace alibabacloud
