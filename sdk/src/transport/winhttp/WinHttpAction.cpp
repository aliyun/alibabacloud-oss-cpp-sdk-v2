#include "WinHttpAction.h"
#include "src/utils/LogUtils.h"

namespace alibabacloud::oss2::transport::winhttp {

static const char* TAG = "WinHttpAction";

WinHttpAction::WinHttpAction()
    : event_(CreateEvent(nullptr, TRUE, FALSE, nullptr)),
      expectedStatus_(0),
      error_(0),
      bytesAvailable_(0),
      bytesRead_(0) {}

WinHttpAction::~WinHttpAction() {
    if (event_ != nullptr) {
        CloseHandle(event_);
    }
}

bool WinHttpAction::registerCallback(HINTERNET hRequest) {
    return WinHttpSetStatusCallback(hRequest, &WinHttpAction::statusCallback,
                                    WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES
                                        | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR,
                                    0)
        != WINHTTP_INVALID_STATUS_CALLBACK;
}

bool WinHttpAction::waitForAction(std::function<bool()> initiateAction, DWORD expectedStatus,
                                  const std::optional<CancellationToken>& token,
                                  const std::function<bool()>& isDisabled) {
    ResetEvent(event_);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = 0;
        bytesAvailable_ = 0;
        bytesRead_ = 0;
    }
    expectedStatus_.store(expectedStatus, std::memory_order_release);

    if (!initiateAction()) {
        return false;
    }

    DWORD waitResult;
    do {
        waitResult = WaitForSingleObject(event_, kPollIntervalMs);
        if (waitResult == WAIT_TIMEOUT) {
            if (token.has_value() && token->isCanceled()) {
                OSS_LOG(LogLevel::LogInfo, TAG, "Request canceled by CancellationToken");
                return false;
            }
            if (isDisabled && isDisabled()) {
                OSS_LOG(LogLevel::LogInfo, TAG, "Request processing disabled");
                return false;
            }
        } else if (waitResult != WAIT_OBJECT_0) {
            OSS_LOG(LogLevel::LogError, TAG, "WaitForSingleObject failed, error: %lu", GetLastError());
            return false;
        }
    } while (waitResult != WAIT_OBJECT_0);

    std::lock_guard<std::mutex> lock(mutex_);
    return error_ == 0;
}

DWORD WinHttpAction::getError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return error_;
}

DWORD WinHttpAction::getBytesAvailable() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bytesAvailable_;
}

DWORD WinHttpAction::getBytesRead() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bytesRead_;
}

void CALLBACK WinHttpAction::statusCallback(HINTERNET /*hInternet*/, DWORD_PTR dwContext, DWORD dwInternetStatus,
                                            LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
    if (dwContext == 0) {
        return;
    }
    auto* action = reinterpret_cast<WinHttpAction*>(dwContext);
    action->onStatus(dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void WinHttpAction::onStatus(DWORD internetStatus, LPVOID statusInfo, DWORD statusInfoLength) {
    if (internetStatus == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR) {
        auto* asyncResult = static_cast<WINHTTP_ASYNC_RESULT*>(statusInfo);
        OSS_LOG(LogLevel::LogError, TAG, "Async request error, API: %lu, error: %lu",
                static_cast<unsigned long>(asyncResult->dwResult), asyncResult->dwError);
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = asyncResult->dwError;
        SetEvent(event_);
        return;
    }

    if (internetStatus == expectedStatus_.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (internetStatus == WINHTTP_CALLBACK_STATUS_READ_COMPLETE) {
            bytesRead_ = statusInfoLength;
        } else if (internetStatus == WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE) {
            bytesAvailable_ = statusInfo ? *static_cast<DWORD*>(statusInfo) : 0;
        }
        SetEvent(event_);
    }
}

} // namespace alibabacloud::oss2::transport::winhttp
