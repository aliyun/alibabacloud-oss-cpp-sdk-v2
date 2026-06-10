#pragma once

#include "alibabacloud/oss2/Config.h"

#ifdef ALIBABACLOUD_OSS_HAS_TINYXML2
#include <tinyxml2.h>
#else
#include "src/thirdparty/tinyxml2/tinyxml2.hpp"
#endif

namespace alibabacloud {
namespace oss2 {

#ifdef ALIBABACLOUD_OSS_HAS_TINYXML2
namespace thirdparty {
namespace tinyxml2 = ::tinyxml2;
}
#endif

} // namespace oss2
} // namespace alibabacloud
