#pragma once

#include "ExecuteMiddleware.h"
#include "alibabacloud/oss2/Error.h"
#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"

#include <system_error>

namespace alibabacloud {
namespace oss2 {
namespace internal {

class StreamObserver {
  public:
    /**
     * Called to indicate that the Stream has been closed.
     */
    virtual void closed() {}

    /**
     * Called to indicate that Stream's read have been called, and are about to invoke data.
     */
    virtual void data(std::uint8_t* buffer, std::size_t count) = 0;

    /**
     * Called to indicate that an error occurred on the underlying stream.
     */
    virtual void error(const std::error_code&) {}

    /**
     * Called to indicate that EOF has been seen on the underlying stream. This method may be called multiple times,
     * if the reader keeps invoking either of the read methods, and they will consequently keep returning EOF.
     */
    virtual void finished() {}

    /**
     * Called to indicate that the state of the underlying stream is reset.
     */
    virtual void reset() {}

    virtual ~StreamObserver() = default;
};


class ProgressObserver : public StreamObserver {
  public:
    ProgressObserver(ProgressCallback callback, std::int64_t total)
        : callback_(callback), total_(total), written_(0), lastWritten_(0) {}

    void data(std::uint8_t* buffer, std::size_t count) override {
        ((void) (buffer));
        written_ += count;
        if (written_ > lastWritten_) {
            callback_(count, written_, total_);
        }
    }

    void reset() override {
        lastWritten_ = written_;
        written_ = 0;
    }

  private:
    ProgressCallback callback_;
    std::int64_t total_;
    std::size_t written_;
    std::size_t lastWritten_;
};


class CRC64Observer : public StreamObserver {
  public:
    CRC64Observer(uint64_t init = 0) : init_(init), value_(init) {}

    void data(std::uint8_t* buffer, std::size_t count) override;

    void reset() override {
        value_ = init_;
    }

    inline uint64_t crc() const {
        return value_;
    }

    inline std::string crcAsString() const {
        return std::to_string(value_);
    }

  private:
    uint64_t init_;
    uint64_t value_;
};

struct CRC64ResponseChecker {
    // cppcheck-suppress constParameterReference
    bool operator()(std::unique_ptr<ResponseMessage>& response, ExecuteContext& context) {
        if (!checker) {
            return true;
        }
        auto it = response->headers.find("x-oss-hash-crc64ecma");
        if (it == response->headers.end()) {
            return true;
        }
        auto ccrc = checker->crcAsString();
        if (ccrc == it->second) {
            return true;
        }
        context.errorContext.error = make_error_code(ClientErrorCode::CrcMismatch);
        context.errorContext.errorFields.emplace("Code", "CRCInconsistent");
        context.errorContext.errorFields.emplace(
            "Message", "crc is inconsistent, client crc:" + ccrc + ", server crc:" + it->second);
        return false;
    }
    std::shared_ptr<CRC64Observer> checker;
};

class TeeByteContent : public ByteContent {
  public:
    TeeByteContent(std::shared_ptr<ByteContent> source, std::vector<std::shared_ptr<StreamObserver>> sinks)
        : source_(std::move(source)), sinks_(std::move(sinks)), first_(true) {}

    std::optional<std::size_t> length() const override {
        return source_->length();
    }
    bool isOneShot() const override {
        return source_->isOneShot();
    }
    // cppcheck-suppress uselessOverride
    std::optional<std::filesystem::path> path() const override {
        // intentionally ignore FileContent path from source_
        return std::nullopt;
    };
    std::unique_ptr<ByteSource> spanSource() override;

  private:
    std::vector<StreamObserver*> spanObserver();
    void resetObserver();
    std::shared_ptr<ByteContent> source_;
    std::vector<std::shared_ptr<StreamObserver>> sinks_;
    bool first_;
};


} // namespace internal
} // namespace oss2
} // namespace alibabacloud