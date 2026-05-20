#pragma once

#include "alibabacloud/oss2/utils/Cancellation.h"

#include <windows.h>
#include <winhttp.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>

namespace alibabacloud::oss2::transport::winhttp {

constexpr DWORD kPollIntervalMs = 800;

class WinHttpAction {
  public:
    WinHttpAction();
    ~WinHttpAction();

    WinHttpAction(const WinHttpAction&) = delete;
    WinHttpAction& operator=(const WinHttpAction&) = delete;

    bool registerCallback(HINTERNET hRequest);

    bool waitForAction(std::function<bool()> initiateAction,
                       DWORD expectedStatus,
                       const std::optional<CancellationToken>& token,
                       const std::function<bool()>& isDisabled = {});

    DWORD getError() const;
    DWORD getBytesAvailable() const;
    DWORD getBytesRead() const;

  private:
    static void CALLBACK statusCallback(
        HINTERNET hInternet, DWORD_PTR dwContext,
        DWORD dwInternetStatus, LPVOID lpvStatusInformation,
        DWORD dwStatusInformationLength);

    void onStatus(DWORD internetStatus, LPVOID statusInfo, DWORD statusInfoLength);

    HANDLE event_;
    std::atomic<DWORD> expectedStatus_;
    DWORD error_;
    DWORD bytesAvailable_;
    DWORD bytesRead_;
    mutable std::mutex mutex_;
};

} // namespace alibabacloud::oss2::transport::winhttp
