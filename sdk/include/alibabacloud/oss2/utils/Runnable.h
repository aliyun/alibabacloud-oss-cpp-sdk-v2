/*
 * Copyright 2009-2017 Alibaba Cloud All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "alibabacloud/oss2/OSS_EXPORTS.h"
#include <functional>


namespace alibabacloud {
namespace oss2 {
class ALIBABACLOUD_OSS_API Runnable {
  public:
    explicit Runnable(const std::function<void()> f) : f_(f) {}
    void run() const {
        f_();
    }

  private:
    std::function<void()> f_;
};
} // namespace oss2
} // namespace alibabacloud