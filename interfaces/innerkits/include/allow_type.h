/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_ALLOW_TYPE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_ALLOW_TYPE_H

#include <vector>
#include <string>

namespace OHOS {
namespace DevStandbyMgr {
struct AllowType {
    enum : uint32_t {
        NETWORK = 1,
        RUNNING_LOCK = 1 << 1,
        TIMER = 1 << 2,
        WORK_SCHEDULER = 1 << 3,
        AUTO_SYNC = 1 << 4,
        PUSH = 1 << 5,
        FREEZE = 1 << 6,
    };
};

extern const std::vector<std::string> AllowTypeName;
extern const uint32_t MAX_ALLOW_TYPE_NUM;
extern const uint32_t MAX_ALLOW_TYPE_NUMBER;
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACES_INNERKITS_INCLUDE_ALLOW_TYPE_H