/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_TIME_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_TIME_INFO_H

#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include <cstdint>
#include <functional>
#include <string>
#include <sys/time.h>
#include <vector>

#include "time_service_client.h"
#include "itimer_info.h"

namespace OHOS {
namespace DevStandbyMgr {
class TimedTask : public MiscServices::ITimerInfo {
public:
    TimedTask();
    TimedTask(bool repeat, uint64_t interval, bool isExact, bool isIdle = false);
    virtual ~TimedTask();
    void OnTrigger() override;
    void SetType(const int &type) override;
    void SetRepeat(bool repeat) override;
    void SetInterval(const uint64_t& interval) override;
    void SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent) override;
    void SetCallbackInfo(const std::function<void()>& callBack);

    static uint64_t CreateTimer(bool repeat, uint64_t interval, bool isExact, bool isIdle,
        const std::function<void()>& callBack);
    static bool StartDayNightSwitchTimer(uint64_t& timeId);
    static bool RegisterDayNightSwitchTimer(uint64_t& timeId, bool repeat, uint64_t interval,
        const std::function<void()>& callBack);
public:
    std::function<void()> callBack_ = nullptr;
};
} // namespace DevStandbyMgr
} // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_TIME_INFO_H