#include "SerdeObjectSymlink.h"
#include "SerdeUtils.h"
#include "alibabacloud/oss2/Error.h"
#include "src/utils/Utils.h"


namespace alibabacloud {
namespace oss2 {
namespace transform {


OperationInput fromPutSymlink(const models::PutSymlinkRequest& request) {
    auto input = OperationInput{"PutSymlink", "PUT"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("symlink", "");


    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::PutSymlinkResult, OperationError> toPutSymlink(OperationOutput&& output) {
    return models::PutSymlinkResult(output.statusCode, std::move(output.headers));
}


OperationInput fromGetSymlink(const models::GetSymlinkRequest& request) {
    auto input = OperationInput{"GetSymlink", "GET"};

    // Default Headers
    input.headers.emplace("Content-Type", "application/xml");

    // Default Parameters

    input.parameters.emplace("symlink", "");


    // headers
    for (const auto& [k, v] : request.getHeaders()) {
        input.headers.insert_or_assign(k, v);
    }

    // parameters
    for (const auto& [k, v] : request.getParameters()) {
        input.parameters.insert_or_assign(k, v);
    }

    // body
    std::string md5 = "1B2M2Y8AsgTpgAmY7PhCfg==";

    input.headers.emplace("Content-MD5", std::move(md5));

    input.bucket = request.getBucket();
    input.key = request.getKey();

    return input;
}

Outcome<models::GetSymlinkResult, OperationError> toGetSymlink(OperationOutput&& output) {
    return models::GetSymlinkResult(output.statusCode, std::move(output.headers));
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud