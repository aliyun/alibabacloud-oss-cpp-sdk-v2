#pragma once

#include "Defaults.h"
#include "ExecuteMiddleware.h"
#include "alibabacloud/oss2/Operation.h"
#include "alibabacloud/oss2/transport/HttpTypes.h"

#include <string>
#include <system_error>

namespace alibabacloud {
namespace oss2 {
namespace internal {

enum class EndpointType { Default, DualStack, Internal, Accelerate, Overseas };

bool isValidIp(const std::string& host);
bool isValidBucketName(const std::string& bucket);
bool isValidObjectName(const std::string& key);
bool isValidMethod(const std::string& key);

std::string addScheme(const std::string& value, bool disableSsl);
std::string regionToEndpoint(const std::string& value, EndpointType type, bool disableSsl);
std::string buildHostPath(const OperationInput& input, const std::string& baseUrl, AddressStyleType addressStyle);

void updateError(ExecuteContext& context, std::error_code errorCode, const char* code, const char* message);
void updateError(ExecuteContext& context, std::error_code errorCode, const char* code, std::string&& message);

} // namespace internal
} // namespace oss2
} // namespace alibabacloud
