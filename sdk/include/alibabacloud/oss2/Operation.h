#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/utils/Cancellation.h"
#include "alibabacloud/oss2/utils/Outcome.h"


#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {


struct ALIBABACLOUD_OSS_API OperationOptions {
    std::optional<long> retryMaxAttempts{};
    std::optional<long> readWriteTimeout{};
    std::optional<CancellationToken> cancellationToken{};
};

struct ALIBABACLOUD_OSS_API OperationInput {
    std::string opName{};
    std::string method{};
    HeaderCollection headers{};
    ParameterCollection parameters{};
    std::optional<std::string> bucket{};
    std::optional<std::string> key{};
    AttributeMap opMetadata{};
    std::shared_ptr<ByteContent> body{};
};

struct ALIBABACLOUD_OSS_API OperationOutput {
    int statusCode{};
    HeaderCollection headers{};
    std::shared_ptr<std::iostream> body{};
};

class ALIBABACLOUD_OSS_API OperationError {
  public:
    OperationError() = default;
    OperationError(std::error_code errorCode, std::map<std::string, std::string> errorFields)
            : errorCode_(std::move(errorCode)), errorFields_(std::move(errorFields)){};

    OperationError(std::string opName, std::string method, std::string requestTarget, std::error_code errorCode,
                   std::map<std::string, std::string> errorFields)
            : opName_(std::move(opName)), method_(std::move(method)), requestTarget_(std::move(requestTarget)),
              errorCode_(std::move(errorCode)), errorFields_(std::move(errorFields)) {}

    const std::string& getCode() const;
    const std::string& getMessage() const;
    const std::string& getEC() const;
    const std::string& getRequestId() const;
    const std::map<std::string, std::string>& getErrorFields() const {
        return errorFields_;
    }
    const std::string& getSnapshot() const {
        return snapshot_;
    };

    const HeaderCollection& getHeaders() const {
        return headers_;
    };

    const std::string& getOpName() const {
        return opName_;
    }

    const std::string& getRequestTarget() const {
        return requestTarget_;
    }

    const std::string& getMethod() const {
        return method_;
    }

    const std::error_code getErrorCode() const {
        return errorCode_;
    }

    int getStatusCode() const {
        return statusCode_;
    }

    std::string toString() const;

    inline void setResponseResult(int statusCode, HeaderCollection&& headers, std::string&& snapshot) {
        statusCode_ = statusCode;
        headers_ = std::move(headers);
        snapshot_ = std::move(snapshot);
    }

  private:
    // request
    std::string opName_;
    std::string method_;
    std::string requestTarget_;

    // error
    std::error_code errorCode_;
    std::map<std::string, std::string> errorFields_;

    // response
    int statusCode_{};
    HeaderCollection headers_;
    std::string snapshot_;
};

// C++23 Expected<T>
using OperationResult = std::variant<OperationOutput, OperationError>;

// Utils for request body
namespace RequestBody {
template <class T>
inline std::shared_ptr<ByteContent> FromString(T&& data) {
    return std::make_shared<StringContent>(std::forward<T>(data));
}

template <class T>
inline std::shared_ptr<ByteContent> FromStream(T&& stream) {
    if (stream == nullptr) {
        return std::make_shared<EmptyContent>();
    }
    return std::make_shared<StreamContent>(std::forward<T>(stream));
}

template <class T>
inline std::shared_ptr<ByteContent> FromFile(T&& file) {
    return std::make_shared<FileContent>(std::forward<T>(file));
}

template <class T>
inline std::shared_ptr<ByteContent> FromMemory(T&& data) {
    return std::make_shared<MemoryContent>(std::forward<T>(data));
}

inline std::shared_ptr<ByteContent> FromMemory(const char* data, std::size_t len) {
    if (data == nullptr) {
        return std::make_shared<EmptyContent>();
    }
    return std::make_shared<MemoryContent>(std::string_view(data, len));
}

} // namespace RequestBody


} // namespace oss2
} // namespace alibabacloud