
#include "WinHttpConnectionPool.h"
#include "src/utils/LogUtils.h"

#include <sstream>

namespace alibabacloud {
namespace oss2 {
namespace transport {
namespace winhttp {

static const char* TAG = "WinHttpConnectionPool";

WinHttpConnectionPool::WinHttpConnectionPool(HINTERNET hSession, unsigned maxConnectionsPerHost,
                                             long connectTimeoutMs, long requestTimeoutMs)
    : hSession_(hSession),
      maxConnectionsPerHost_(maxConnectionsPerHost),
      connectTimeoutMs_(connectTimeoutMs),
      requestTimeoutMs_(requestTimeoutMs) {
}

WinHttpConnectionPool::~WinHttpConnectionPool() {
    Cleanup();
}

std::string WinHttpConnectionPool::MakeKey(const std::string& host, unsigned port) {
    std::string key = host;
    key.append(":");
    key.append(std::to_string(port));
    return key;
}

HINTERNET WinHttpConnectionPool::CreateNewConnection(const std::string& host, HostConnectionContainer& container) {
    std::wstring wHost(host.begin(), host.end());
    HINTERNET hConnect = WinHttpConnect(hSession_, wHost.c_str(), container.port, 0);
    if (hConnect == nullptr) {
        OSS_LOG(LogLevel::LogError, TAG, "Failed to create connection to %s:%u, error: %lu",
                host.c_str(), container.port, GetLastError());
        return nullptr;
    }

    DWORD timeoutMs = static_cast<DWORD>(connectTimeoutMs_);
    DWORD requestMs = static_cast<DWORD>(requestTimeoutMs_);
    WinHttpSetOption(hConnect, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
    WinHttpSetOption(hConnect, WINHTTP_OPTION_RECEIVE_TIMEOUT, &requestMs, sizeof(requestMs));

    return hConnect;
}

bool WinHttpConnectionPool::CheckAndGrowPool(const std::string& host, HostConnectionContainer& container) {
    std::lock_guard<std::mutex> locker(containerLock_);
    if (container.currentPoolSize < maxConnectionsPerHost_) {
        unsigned multiplier = container.currentPoolSize > 0 ? container.currentPoolSize : 1;
        unsigned amountToAdd = (std::min)(multiplier * 2, maxConnectionsPerHost_ - container.currentPoolSize);
        unsigned actuallyAdded = 0;

        for (unsigned i = 0; i < amountToAdd; ++i) {
            HINTERNET newConnection = CreateNewConnection(host, container);
            if (newConnection != nullptr) {
                container.hostConnections.Release(newConnection);
                ++actuallyAdded;
            } else {
                break;
            }
        }

        container.currentPoolSize += actuallyAdded;
        return actuallyAdded > 0;
    }
    return false;
}

HINTERNET WinHttpConnectionPool::AcquireConnection(const std::string& host, uint16_t port) {
    std::string key = MakeKey(host, port);
    HostConnectionContainer* container = nullptr;

    {
        std::lock_guard<std::mutex> locker(hostConnectionsMutex_);
        auto it = hostConnections_.find(key);
        if (it != hostConnections_.end()) {
            container = it->second;
        } else {
            container = new HostConnectionContainer();
            container->currentPoolSize = 0;
            container->port = port;
            hostConnections_[key] = container;
        }
    }

    if (!container->hostConnections.HasResourcesAvailable()) {
        CheckAndGrowPool(host, *container);
    }

    return container->hostConnections.Acquire();
}

void WinHttpConnectionPool::ReleaseConnection(const std::string& host, unsigned port, HINTERNET hConnect) {
    if (hConnect == nullptr) return;

    std::string key = MakeKey(host, port);
    std::lock_guard<std::mutex> locker(hostConnectionsMutex_);
    auto it = hostConnections_.find(key);
    if (it != hostConnections_.end()) {
        it->second->hostConnections.Release(hConnect);
    }
}

void WinHttpConnectionPool::Cleanup() {
    for (auto& [key, container] : hostConnections_) {
        auto handles = container->hostConnections.ShutdownAndWait(container->currentPoolSize);
        for (HINTERNET handle : handles) {
            WinHttpCloseHandle(handle);
        }
        delete container;
    }
    hostConnections_.clear();
}

} // namespace winhttp
} // namespace transport
} // namespace oss2
} // namespace alibabacloud
