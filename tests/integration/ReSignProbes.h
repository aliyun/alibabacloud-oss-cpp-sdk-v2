#pragma once

#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/transport/HttpTransport.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <ios>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace alibabacloud {
namespace oss2 {
namespace test {

/// Normalizes a failed attempt to a retryable error, since transports report a
/// failed body read with different codes.
inline TransportError asRetryableError(const TransportError& error) {
    return TransportError{make_error_code(TransportErrorCode::SendRecvError), error.errorCode, error.errorMessage};
}

/// Wraps the real HTTP transport and records the headers of every attempt.
class CapturingHttpTransport final : public HttpTransport {
  public:
    explicit CapturingHttpTransport(std::shared_ptr<HttpTransport> inner) : inner_(std::move(inner)) {}

    ResponseResult send(std::unique_ptr<RequestMessage>& request, const RequestOptions& options) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sentHeaders_.push_back(request->headers);
        }
        auto result = inner_->send(request, options);
        if (auto* error = std::get_if<TransportError>(&result)) {
            return asRetryableError(*error);
        }
        return result;
    }

    std::string getName() const override {
        return "CapturingHttpTransport/" + inner_->getName();
    }

    std::vector<HeaderCollection> getSentHeaders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sentHeaders_;
    }

  private:
    std::shared_ptr<HttpTransport> inner_;
    mutable std::mutex mutex_;
    std::vector<HeaderCollection> sentHeaders_;
};

/// Async counterpart of CapturingHttpTransport.
class CapturingAsyncHttpTransport final : public AsyncHttpTransport {
  public:
    explicit CapturingAsyncHttpTransport(std::shared_ptr<AsyncHttpTransport> inner) : inner_(std::move(inner)) {}

    void sendAsync(std::unique_ptr<RequestMessage> request, const RequestOptions& options,
                   RequestCallback callback) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sentHeaders_.push_back(request->headers);
        }
        inner_->sendAsync(std::move(request), options,
                          [callback = std::move(callback)](ResponseResult result,
                                                           std::unique_ptr<RequestMessage> sentRequest) {
                              if (auto* error = std::get_if<TransportError>(&result)) {
                                  callback(asRetryableError(*error), std::move(sentRequest));
                                  return;
                              }
                              callback(std::move(result), std::move(sentRequest));
                          });
    }

    std::string getName() const override {
        return "CapturingAsyncHttpTransport/" + inner_->getName();
    }

    std::vector<HeaderCollection> getSentHeaders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sentHeaders_;
    }

  private:
    std::shared_ptr<AsyncHttpTransport> inner_;
    mutable std::mutex mutex_;
    std::vector<HeaderCollection> sentHeaders_;
};

/// A replayable body whose first cursor stops half way while length() keeps
/// advertising the full size, so the first attempt fails and is retried.
class TruncateFirstAttemptContent final : public ByteContent {
  public:
    explicit TruncateFirstAttemptContent(std::string content) : content_(std::move(content)) {}

    std::optional<std::size_t> length() const override {
        return content_.size();
    }

    bool isOneShot() const override {
        return false;
    }

    std::unique_ptr<ByteSource> spanSource() override {
        auto limit = spanCount_.fetch_add(1) == 0 ? content_.size() / 2 : content_.size();
        return std::make_unique<Source>(content_, limit);
    }

    int getSpanCount() const {
        return spanCount_.load();
    }

  private:
    class Source final : public ByteSource {
      public:
        Source(const std::string& content, std::size_t limit) : content_(content), limit_(limit) {}

      private:
        std::size_t onRead(std::uint8_t* buffer, std::size_t count) override {
            auto got = std::min(count, limit_ - pos_);
            if (got > 0) {
                std::memcpy(buffer, content_.data() + pos_, got);
                pos_ += got;
            }
            return got;
        }

        int iostate() override {
            return pos_ >= limit_ ? std::ios::eofbit : 0;
        }

        const std::string& content_;
        std::size_t limit_;
        std::size_t pos_{0};
    };

    std::string content_;
    std::atomic<int> spanCount_{0};
};

} // namespace test
} // namespace oss2
} // namespace alibabacloud
