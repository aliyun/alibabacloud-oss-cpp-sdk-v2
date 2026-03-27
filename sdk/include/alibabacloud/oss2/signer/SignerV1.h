
#pragma once

#include <memory>
#include <string>

#include "alibabacloud/oss2/signer/Signer.h"

namespace alibabacloud {
namespace oss2 {

class ALIBABACLOUD_OSS_API SignerV1 : public Signer {
  public:
    bool sign(SigningContext&) override;
    std::string getName() const override {
        return "v1";
    };
};
} // namespace oss2
} // namespace alibabacloud
