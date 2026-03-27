
#pragma once

#include <memory>
#include <string>

#include "alibabacloud/oss2/signer/SigningContext.h"

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API Signer {
  public:
    virtual bool sign(SigningContext& signingContext) = 0;
    virtual std::string getName() const = 0;
    virtual ~Signer() = default;
};
} // namespace oss2
} // namespace alibabacloud
