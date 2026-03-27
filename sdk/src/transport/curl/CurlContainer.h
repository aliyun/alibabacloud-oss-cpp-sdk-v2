
#pragma once

#include <curl/curl.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace alibabacloud::oss2::transport::curl {


////////////////////////////////////////////////////////////////////////////////////////////
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


class CurlContainer {
  public:
    CurlContainer(unsigned maxSize = 16, long requestTimeout = 10000, long connectTimeout = 5000)
            : maxPoolSize_(maxSize), requestTimeout_(requestTimeout), connectTimeout_(connectTimeout), poolSize_(0) {}

    ~CurlContainer() {
        for (CURL* handle : handleContainer_.ShutdownAndWait(poolSize_)) {
            curl_easy_cleanup(handle);
        }
    }

    CURL* Acquire() {
        if (!handleContainer_.HasResourcesAvailable()) {
            growPool();
        }
        CURL* handle = handleContainer_.Acquire();
        return handle;
    }

    void Release(CURL* handle, bool force) {
        if (handle) {
            curl_easy_reset(handle);
            if (force) {
                CURL* newhandle = curl_easy_init();
                if (newhandle) {
                    curl_easy_cleanup(handle);
                    handle = newhandle;
                }
            }
            setDefaultOptions(handle);
            handleContainer_.Release(handle);
        }
    }

  private:
    CurlContainer(const CurlContainer&) = delete;
    const CurlContainer& operator=(const CurlContainer&) = delete;
    CurlContainer(const CurlContainer&&) = delete;
    const CurlContainer& operator=(const CurlContainer&&) = delete;

    bool growPool() {
        std::lock_guard<std::mutex> locker(containerLock_);
        if (poolSize_ < maxPoolSize_) {
            unsigned multiplier = poolSize_ > 0 ? poolSize_ : 1;
            unsigned amountToAdd = (std::min)(multiplier * 2, maxPoolSize_ - poolSize_);

            unsigned actuallyAdded = 0;
            for (unsigned i = 0; i < amountToAdd; ++i) {
                CURL* curlHandle = curl_easy_init();
                if (curlHandle) {
                    setDefaultOptions(curlHandle);
                    handleContainer_.Release(curlHandle);
                    ++actuallyAdded;
                } else {
                    break;
                }
            }
            poolSize_ += actuallyAdded;
            return actuallyAdded > 0;
        }
        return false;
    }

    void setDefaultOptions(CURL* handle) {
        curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(handle, CURLOPT_TCP_NODELAY, 1);
        curl_easy_setopt(handle, CURLOPT_NETRC, CURL_NETRC_IGNORED);

        curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, 0L);
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout_);
        curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, requestTimeout_ / 1000);

        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    }

  private:
    ResourceManager<CURL*> handleContainer_;
    unsigned maxPoolSize_;
    unsigned long requestTimeout_;
    unsigned long connectTimeout_;
    unsigned poolSize_;
    std::mutex containerLock_;
};

} // namespace alibabacloud::oss2::transport::curl
