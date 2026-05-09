
#pragma once

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace transport {
namespace winhttp {

template <typename RESOURCE_TYPE>
class ResourceManager {
  public:
    ResourceManager() : m_shutdown(false) {}
    RESOURCE_TYPE Acquire() {
        std::unique_lock<std::mutex> locker(m_queueLock);
        while (!m_shutdown.load() && m_resources.size() == 0) {
            m_semaphore.wait(locker, [&]() { return m_shutdown.load() || m_resources.size() > 0; });
        }

        assert(!m_shutdown.load());

        RESOURCE_TYPE resource = m_resources.back();
        m_resources.pop_back();

        return resource;
    }

    bool HasResourcesAvailable() {
        std::lock_guard<std::mutex> locker(m_queueLock);
        return m_resources.size() > 0 && !m_shutdown.load();
    }

    void Release(RESOURCE_TYPE resource) {
        std::unique_lock<std::mutex> locker(m_queueLock);
        m_resources.push_back(resource);
        locker.unlock();
        m_semaphore.notify_one();
    }

    void PutResource(RESOURCE_TYPE resource) {
        m_resources.push_back(resource);
    }

    std::vector<RESOURCE_TYPE> ShutdownAndWait(size_t resourceCount) {
        std::vector<RESOURCE_TYPE> resources;
        std::unique_lock<std::mutex> locker(m_queueLock);
        m_shutdown = true;
        while (m_resources.size() < resourceCount) {
            m_semaphore.wait(locker, [&]() { return m_resources.size() == resourceCount; });
        }
        resources = m_resources;
        m_resources.clear();
        return resources;
    }

  private:
    std::vector<RESOURCE_TYPE> m_resources;
    std::mutex m_queueLock;
    std::condition_variable m_semaphore;
    std::atomic<bool> m_shutdown;
};


struct HostConnectionContainer {
    uint16_t port;
    ResourceManager<HINTERNET> hostConnections;
    unsigned currentPoolSize;
};


class WinHttpConnectionPool {
  public:
    WinHttpConnectionPool(HINTERNET hSession, unsigned maxConnectionsPerHost,
                          long connectTimeoutMs, long requestTimeoutMs);
    ~WinHttpConnectionPool();

    HINTERNET AcquireConnection(const std::string& host, uint16_t port);
    void ReleaseConnection(const std::string& host, unsigned port, HINTERNET hConnect);

    void Cleanup();

  private:
    WinHttpConnectionPool(const WinHttpConnectionPool&) = delete;
    const WinHttpConnectionPool& operator=(const WinHttpConnectionPool&) = delete;

    HINTERNET CreateNewConnection(const std::string& host, HostConnectionContainer& container);
    bool CheckAndGrowPool(const std::string& host, HostConnectionContainer& container);

    std::string MakeKey(const std::string& host, unsigned port);

    HINTERNET hSession_;
    unsigned maxConnectionsPerHost_;
    long connectTimeoutMs_;
    long requestTimeoutMs_;
    std::map<std::string, HostConnectionContainer*> hostConnections_;
    std::mutex hostConnectionsMutex_;
    std::mutex containerLock_;
};

} // namespace winhttp
} // namespace transport
} // namespace oss2
} // namespace alibabacloud
