#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"

#include <map>
#include <string>

namespace alibabacloud {
namespace oss2 {
namespace utils {

/**
 * @brief Adds custom MIME type mappings.
 *
 * User-defined mappings take priority over built-in defaults when
 * the SDK auto-detects Content-Type from the object name.
 * If a mapping for a given extension already exists in user mappings,
 * it will be overwritten.
 *
 * This function is NOT thread-safe. Call it before creating any
 * OSSClient instances, or ensure external synchronization.
 *
 * @param mappings A map of file extension (without leading dot, e.g. "json")
 *                 to MIME type (e.g. "application/json").
 *
 * @code
 *   utils::addMimeType({{"parquet", "application/x-parquet"},
 *                       {"avro", "application/avro"}});
 * @endcode
 */
ALIBABACLOUD_OSS_API void addMimeType(const std::map<std::string, std::string>& mappings);

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
