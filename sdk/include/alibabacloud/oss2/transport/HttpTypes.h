#pragma once

#include "alibabacloud/oss2/Types.h"
#include "alibabacloud/oss2/io/ByteStream.h"
#include "alibabacloud/oss2/utils/Cancellation.h"

namespace alibabacloud {
namespace oss2 {

enum class HttpMethod { Get, Head, Post, Put, Delete, Connect, Options, Patch, Trace };

struct ALIBABACLOUD_OSS_API RequestOptions {
    std::optional<OStreamFactory> ostreamFactory;
    std::optional<CancellationToken> cancellationToken;
};

struct ALIBABACLOUD_OSS_API TransportError {
    std::error_code error;
    std::string errorCode;
    std::string errorMessage;
};

struct ALIBABACLOUD_OSS_API RequestMessage {
    std::string method;
    std::string uri;
    HeaderCollection headers;
    std::shared_ptr<ByteContent> body;
};

struct ALIBABACLOUD_OSS_API ResponseMessage {
    long statusCode;
    std::string reason;
    HeaderCollection headers;
    std::shared_ptr<std::iostream> body;
};

// C++23 Expected<T>
// using ResponseResult = std::expected<std::unique_ptr<ResponseMessage>, std::error_code>;
using ResponseResult = std::variant<std::unique_ptr<ResponseMessage>, TransportError>;

using RequestCallback = std::function<void(ResponseResult, std::unique_ptr<RequestMessage>)>;

} // namespace oss2
} // namespace alibabacloud