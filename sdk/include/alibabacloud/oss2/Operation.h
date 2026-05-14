#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/utils/Cancellation.h"
#include "alibabacloud/oss2/utils/Outcome.h"

#include <functional>
#include <optional>
#include <string>

namespace alibabacloud {
namespace oss2 {


/**
 * @brief Per-request options that override client-level defaults.
 *
 * Pass an OperationOptions to any OSSClient method to customize behavior
 * for that single request without affecting other requests.
 *
 * @code
 *   OperationOptions opts;
 *   opts.retryMaxAttempts = 5;
 *   opts.readWriteTimeout = 30000;  // 30 seconds
 *   auto result = client.putObject(request, &opts);
 * @endcode
 */
struct ALIBABACLOUD_OSS_API OperationOptions {
    /// Maximum number of retry attempts for this request.
    /// If not set, falls back to the Retryer's default from ClientConfiguration.
    std::optional<long> retryMaxAttempts{};

    /// Read/write timeout in milliseconds for this request.
    /// If not set, falls back to the client-level default.
    std::optional<long> readWriteTimeout{};

    /// Cancellation token to cancel this request while in progress.
    std::optional<CancellationToken> cancellationToken{};
};

/**
 * @brief Low-level, type-erased representation of an OSS API request.
 *
 * OperationInput is the wire-format input consumed by ClientImpl::Execute().
 * Each strongly-typed request (e.g., PutObjectRequest) is converted into an
 * OperationInput by the corresponding transform::from*() function in the
 * serialization layer.
 *
 * Users can also construct an OperationInput directly and pass it to
 * OSSClient::invokeOperation() to call OSS APIs that do not yet have a
 * strongly-typed wrapper.
 *
 * @code
 *   OperationInput input;
 *   input.opName = "PutObject";
 *   input.method = "PUT";
 *   input.bucket = "my-bucket";
 *   input.key    = "my-key";
 *   input.body   = RequestBody::FromString("hello");
 *   auto result  = client.invokeOperation(input);
 * @endcode
 */
struct ALIBABACLOUD_OSS_API OperationInput {
    /// The API operation name (e.g., "PutObject", "GetObject").
    /// Used for logging, error messages, and signing context.
    std::string opName{};

    /// HTTP method: "GET", "PUT", "POST", "DELETE", or "HEAD".
    std::string method{};

    /// HTTP headers to send with the request.
    /// The SDK automatically adds Host, User-Agent, Date, and Authorization.
    HeaderCollection headers{};

    /// URL query parameters (e.g., {"versionId", "xxx"}).
    ParameterCollection parameters{};

    /// Target bucket name. Used for endpoint construction (virtual-hosted
    /// or path-style) and request signing.
    std::optional<std::string> bucket{};

    /// Target object key. Used for URL path construction and signing.
    std::optional<std::string> key{};

    /// Internal metadata for the operation (e.g., presign expiration time).
    /// Not sent over the wire; consumed by the SDK middleware.
    AttributeMap opMetadata{};

    /// The request body. nullptr for bodiless requests (GET, HEAD, DELETE).
    /// Use the RequestBody helpers (FromString, FromFile, etc.) to create.
    std::shared_ptr<ByteContent> body{};
};

/**
 * @brief Low-level, type-erased representation of an OSS API response.
 *
 * Produced by ClientImpl::Execute() on success. Each strongly-typed result
 * (e.g., PutObjectResult) is constructed from an OperationOutput by the
 * corresponding transform::to*() function.
 */
struct ALIBABACLOUD_OSS_API OperationOutput {
    /// HTTP status code (e.g., 200, 204, 404).
    int statusCode{};

    /// Response headers from the OSS server.
    HeaderCollection headers{};

    /// Response body stream. nullptr for bodiless responses (HEAD, DELETE).
    /// For GetObject, this contains the object data.
    std::shared_ptr<std::iostream> body{};
};

/**
 * @brief Describes an error that occurred during an OSS operation.
 *
 * OperationError captures both client-side errors (validation failures,
 * signing errors, network errors) and server-side errors (HTTP 4xx/5xx
 * responses with XML error bodies). It is used as the error variant of
 * OperationResult and the various Outcome types.
 *
 * Error details are stored in two complementary forms:
 *   - errorCode_:   a std::error_code for programmatic error classification
 *   - errorFields_: a string map with "Code", "Message", "RequestId", etc.
 *                    parsed from the OSS XML error response
 *
 * @code
 *   auto result = client.putObject(request);
 *   if (std::holds_alternative<OperationError>(result)) {
 *       auto& err = std::get<OperationError>(result);
 *       std::cerr << "Operation " << err.getOpName() << " failed: "
 *                 << err.getCode() << " - " << err.getMessage()
 *                 << " (RequestId: " << err.getRequestId() << ")"
 *                 << std::endl;
 *   }
 * @endcode
 */
class ALIBABACLOUD_OSS_API OperationError {
  public:
    OperationError() = default;

    /// Constructs an error from a client-side error code and detail fields.
    OperationError(std::error_code errorCode, std::map<std::string, std::string> errorFields)
            : errorCode_(std::move(errorCode)), errorFields_(std::move(errorFields)) {};

    /// Constructs a full error with request context and error details.
    OperationError(std::string opName, std::string method, std::string requestTarget, std::error_code errorCode,
                   std::map<std::string, std::string> errorFields)
            : opName_(std::move(opName)), method_(std::move(method)), requestTarget_(std::move(requestTarget)),
              errorCode_(std::move(errorCode)), errorFields_(std::move(errorFields)) {}

    /// Returns the OSS error code string (e.g., "NoSuchKey", "AccessDenied").
    const std::string& getCode() const;

    /// Returns the human-readable error message from the server.
    const std::string& getMessage() const;

    /// Returns the OSS error code (EC) field, if present.
    const std::string& getEC() const;

    /// Returns the server-assigned request ID for troubleshooting.
    const std::string& getRequestId() const;

    /// Returns all error detail fields parsed from the XML error body.
    const std::map<std::string, std::string>& getErrorFields() const {
        return errorFields_;
    }

    /// Returns the raw XML/text error body snapshot for diagnostics.
    const std::string& getSnapshot() const {
        return snapshot_;
    };

    /// Returns the HTTP response headers, if a response was received.
    const HeaderCollection& getHeaders() const {
        return headers_;
    };

    /// Returns the API operation name (e.g., "PutObject").
    const std::string& getOpName() const {
        return opName_;
    }

    /// Returns the full request URI that was attempted.
    const std::string& getRequestTarget() const {
        return requestTarget_;
    }

    /// Returns the HTTP method used (e.g., "PUT", "GET").
    const std::string& getMethod() const {
        return method_;
    }

    /// Returns the std::error_code for programmatic error handling.
    const std::error_code getErrorCode() const {
        return errorCode_;
    }

    /// Returns the HTTP status code (e.g., 403, 404), or 0 if no response.
    int getStatusCode() const {
        return statusCode_;
    }

    /// Returns a formatted string with all error details for logging.
    std::string toString() const;

    /// @internal Populates response-level fields after receiving an HTTP response.
    inline void setResponseResult(int statusCode, HeaderCollection&& headers, std::string&& snapshot) {
        statusCode_ = statusCode;
        headers_ = std::move(headers);
        snapshot_ = std::move(snapshot);
    }

  private:
    // request context
    std::string opName_;
    std::string method_;
    std::string requestTarget_;

    // error details
    std::error_code errorCode_;
    std::map<std::string, std::string> errorFields_;

    // response context (populated only when a server response was received)
    int statusCode_{};
    HeaderCollection headers_;
    std::string snapshot_;
};

/**
 * @brief The result type for OSSClient::invokeOperation().
 *
 * A variant that holds either a successful OperationOutput or an
 * OperationError. Mirrors the semantics of C++23 std::expected<T, E>.
 *
 * @code
 *   auto result = client.invokeOperation(input);
 *   if (std::holds_alternative<OperationError>(result)) {
 *       // handle error
 *   } else {
 *       auto& output = std::get<OperationOutput>(result);
 *       // use output.statusCode, output.headers, output.body
 *   }
 * @endcode
 */
using OperationResult = std::variant<OperationOutput, OperationError>;

using OperationCallback = std::function<void(OperationResult)>;

/**
 * @brief Convenience factories for constructing request body ByteContent.
 *
 * These helpers create the appropriate ByteContent subclass and return it
 * as a shared_ptr<ByteContent>, ready to assign to a request's body field.
 *
 * @code
 *   // Owned string body (data is copied/moved in)
 *   request.setBody(RequestBody::FromString("<xml>...</xml>"));
 *
 *   // File body (opens file on each retry)
 *   request.setBody(RequestBody::FromFile("/path/to/data.bin"));
 *
 *   // Stream body (shared ownership of the istream)
 *   auto ifs = std::make_shared<std::ifstream>("data.bin", std::ios::binary);
 *   request.setBody(RequestBody::FromStream(ifs));
 *
 *   // Non-owning memory body (zero-copy; caller must keep data alive)
 *   request.setBody(RequestBody::FromMemory(buf, bufLen));
 * @endcode
 */
namespace RequestBody {

/// Creates a StringContent that owns a copy of @p data.
template <class T>
inline std::shared_ptr<ByteContent> FromString(T&& data) {
    return std::make_shared<StringContent>(std::forward<T>(data));
}

/// Creates a StreamContent from a shared istream. Returns EmptyContent if null.
template <class T>
inline std::shared_ptr<ByteContent> FromStream(T&& stream) {
    if (stream == nullptr) {
        return std::make_shared<EmptyContent>();
    }
    return std::make_shared<StreamContent>(std::forward<T>(stream));
}

/// Creates a FileContent from a file path (string or std::filesystem::path).
template <class T>
inline std::shared_ptr<ByteContent> FromFile(T&& file) {
    return std::make_shared<FileContent>(std::forward<T>(file));
}

/// Creates a non-owning MemoryContent from a string_view or similar type.
/// @warning The caller MUST ensure the referenced memory outlives the request.
/// If you need owning semantics, use FromString() instead.
template <class T>
inline std::shared_ptr<ByteContent> FromMemory(T&& data) {
    return std::make_shared<MemoryContent>(std::forward<T>(data));
}

/// Creates a non-owning MemoryContent from a raw pointer and length.
/// Returns EmptyContent if @p data is nullptr.
/// @warning The caller MUST ensure the referenced memory outlives the request.
inline std::shared_ptr<ByteContent> FromMemory(const char* data, std::size_t len) {
    if (data == nullptr) {
        return std::make_shared<EmptyContent>();
    }
    return std::make_shared<MemoryContent>(std::string_view(data, len));
}

} // namespace RequestBody


} // namespace oss2
} // namespace alibabacloud