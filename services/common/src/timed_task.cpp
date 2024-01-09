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

#include "timed_task.h"

#include "standby_service_log.h"
#include "common_constant.h"
#include "time_provider.h"
#include "standby_config_manager.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr int32_t LOW_DELAY_TIME_INTERVAL = 0;
    constexpr int32_t HIGH_DELAY_TIME_INTERVAL = 15 * 60 * 1000;
}
TimedTask::TimedTask()
{}

TimedTask::TimedTask(bool repeat, uint64_t interval, bool isExact, bool isIdle)
{
    this->repeat = repeat;
    this->interval = interval;
    this->type = TIMER_TYPE_WAKEUP;
    if (isExact) {
        this->type = TIMER_TYPE_WAKEUP + TIMER_TYPE_EXACT;
    }
    if (StandbyConfigManager::GetInstance()->GetStandbySwitch(S3_SWITCH)) {
        this->type = TIMER_TYPE_EXACT;
    }
    if (isIdle) {
        this->type = TIMER_TYPE_IDLE;
    }
}

TimedTask::~TimedTask()
{}

void TimedTask::OnTrigger()
{
    STANDBYSERVICE_LOGD("timed task had been triggered");
    if (callBack_ != nullptr) {
        STANDBYSERVICE_LOGD("start invoke callback function of timed task");
        callBack_();
    }
    STANDBYSERVICE_LOGD("end timed task callback");
}

void TimedTask::SetType(const int &type)
{
    this->type = type;
}

void TimedTask::SetRepeat(bool repeat)
{
    this->repeat = repeat;
}

void TimedTask::SetInterval(const uint64_t& interval)
{
    this->interval = interval;
}

void TimedTask::SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    this->wantAgent = wantAgent;
}

void TimedTask::SetCallbackInfo(const std::function<void()>& callBack)
{
    this->callBack_ = callBack;
}

uint64_t WEAK_FUNC TimedTask::CreateTimer(bool repeat, uint64_t interval, bool isExact, bool isIdle,
    const std::function<void()>& callBack)
{
    auto timedTask = std::make_shared<TimedTask>(repeat, interval, isExact, isIdle);
    timedTask->SetCallbackInfo(callBack);
    return MiscServices::TimeServiceClient::GetInstance()->CreateTimer(timedTask);
}

bool WEAK_FUNC TimedTask::StartDayNightSwitchTimer(uint64_t& timeId)
{
    int64_t timeDiff {0};
    if (!TimeProvider::TimeDiffToDayNightSwitch(timeDiff)) {
        return false;
    }
    timeDiff += TimeProvider::GetRandomDelay(LOW_DELAY_TIME_INTERVAL, HIGH_DELAY_TIME_INTERVAL);
    STANDBYSERVICE_LOGI("start next day and night switch after " SPUBI64 " ms", timeDiff);

    auto curTimeStamp = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();
    if (!MiscServices::TimeServiceClient::GetInstance()->StartTimer(timeId, curTimeStamp + timeDiff)) {
        STANDBYSERVICE_LOGE("day and night switch observer start failed");
        return false;
    }
    return true;
}

bool WEAK_FUNC TimedTask::RegisterDayNightSwitchTimer(uint64_t& timeId, bool repeat, uint64_t interval,
    const std::function<void()>& callBack)
{
    timeId = CreateTimer(repeat, interval, false, false, callBack);
    if (timeId == 0) {
        STANDBYSERVICE_LOGE("create timer failed");
        return false;
    }
    return StartDayNightSwitchTimer(timeId);
}
} // namespace DevStandbyMgr
} // namespace OHOS