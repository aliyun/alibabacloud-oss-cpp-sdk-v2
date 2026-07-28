#pragma once

#include "alibabacloud/oss2/ClientOptions.h"

#include <string>

namespace alibabacloud {
namespace oss2 {
namespace agentic {

// Builds the functional options shared by the sync and async agentic clients:
// a bucket-name resolver that expands the logical bucket into its physical
// {bucket}-{accountId}-{region}{suffix} form, and an endpoint provider that
// routes requests to the corresponding virtual-hosted (or path-style) host.
ClientOptionsFns makeAgenticOptionsFns(const std::string& accountId, const std::string& region,
                                       const std::string& suffix);

} // namespace agentic
} // namespace oss2
} // namespace alibabacloud
