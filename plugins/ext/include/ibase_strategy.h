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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_IBASE_STRATEGY_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_IBASE_STRATEGY_H

#include "standby_service_errors.h"
#include "standby_messsage.h"

namespace OHOS {
namespace DevStandbyMgr {
class ExemptionTypeFlag {
public:
enum : uint8_t {
    // apps which has applied continuous task
    CONTINUOUS_TASK = 1,
    // apps which has applied transient task
    TRANSIENT_TASK = 1 << 1,
    // app with work scheduler
    WORK_SCHEDULER = 1 << 2,
    // foreground app will not be restricted
    FOREGROUND_APP = 1 << 3,
    // default exemption, used for system app or native process
    UNRESTRICTED = 1 << 4,
    // applied exemption, used for app configured in exemption list or applied exemption
    EXEMPTION = 1 << 5,
    // app is configured to restricted
    RESTRICTED = 1 << 6,
};

public:
    inline static bool IsExempted(uint8_t flag)
    {
        if ((flag & EXEMPTION) != 0) {
            return true;
        } else if ((flag & RESTRICTED) != 0) {
            return false;
        }
        return flag != 0;
    }
};

struct RestrictedAppInfo {
    std::string name_ {""};
    int32_t uid_ {-1};
    int32_t pid_ {-1};
    uint8_t appExemptionFlag_ {0};
};

struct RestrictedProcInfo {
    std::string name_ {""};
    int32_t uid_ {-1};
    std::set<int32_t> pids_ {};
    uint8_t appExemptionFlag_ {0};
};

class IBaseStrategy {
public:
    virtual void HandleEvent(const StandbyMessage& message) = 0;
    /**
     * @brief invoked when strategy is initialized, reset restriction status
     */
    virtual ErrCode OnCreated() = 0;
    /**
     * @brief invoked when strategy is destroyed, finalize restriction status
     */
    virtual ErrCode OnDestroy() = 0;
    virtual void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) = 0;
    virtual ~IBaseStrategy() = default;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_IBASE_STRATEGY_H
