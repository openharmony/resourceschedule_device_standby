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

#include "time_provider.h"

#include <random>
#include "time_service_client.h"
#include "timed_task.h"
#include "standby_service_log.h"
#include "standby_config_manager.h"
#ifdef STANDBY_POWER_MANAGER_ENABLE
#include "power_mode_info.h"
#include "power_mgr_client.h"
#endif

namespace OHOS {
namespace DevStandbyMgr {
namespace {
    constexpr int32_t NIGHT_ENTRANCE_HOUR = 23;
    constexpr int32_t NIGHT_ENTRANCE_MIN = 45;
    constexpr int32_t DAY_ENTRANCE_HOUR = 6;
    constexpr int32_t DAY_ENTRANCE_MIN = 0;
    constexpr int32_t TWENTY_MIN_ENTRANCE_MIN = 20;
    constexpr int32_t QUARTER_MIN_ENTRANCE_MIN = 15;
    constexpr int32_t TEN_MIN_ENTRANCE_MIN = 10;
    constexpr int32_t NIGHT_TWENTY_TWO_CLOCK = 22;
    constexpr int32_t NIGHT_TWENTY_THREE_CLOCK = 23;
    constexpr int32_t NIGHT_FIVE_CLOCK = 5;
    constexpr int32_t THREE_QUARTERS = 45;
#ifdef STANDBY_POWER_MANAGER_ENABLE
    constexpr int32_t SAVE_MODE_ENTRANCE_MIN = 1;
#endif
}

bool TimeProvider::ConvertTimeStampToLocalTime(int64_t curTimeStamp, struct tm& curLocalTime)
{
    auto res = localtime_r(&curTimeStamp, &curLocalTime);
    if (res == nullptr) {
        STANDBYSERVICE_LOGE("localtime_r failed, can not get valid local time");
        return false;
    }
    return true;
}

uint32_t TimeProvider::GetCondition(int64_t afterNextSeconds)
{
    int64_t curSecTimeStamp = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs() / MSEC_PER_SEC;
    struct tm curLocalTime {};
    curSecTimeStamp += afterNextSeconds;
    if (!ConvertTimeStampToLocalTime(curSecTimeStamp, curLocalTime)) {
        STANDBYSERVICE_LOGE("convert time stamp to local time failed");
        return ConditionType::DAY_STANDBY;
    }
    STANDBYSERVICE_LOGD("current local time info: %{public}02d:%{public}02d:%{public}02d", curLocalTime.tm_hour,
        curLocalTime.tm_min, curLocalTime.tm_sec);
    if ((curLocalTime.tm_hour < NIGHT_ENTRANCE_HOUR || curLocalTime.tm_min < NIGHT_ENTRANCE_MIN) &&
        (curLocalTime.tm_hour >= DAY_ENTRANCE_HOUR)) {
        return ConditionType::DAY_STANDBY;
    }
    return ConditionType::NIGHT_STANDBY;
}

bool TimeProvider::TimeDiffToDayNightSwitch(int64_t& timeDiff)
{
    int64_t curSecTimeStamp = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs() / MSEC_PER_SEC;
    bool res {false};
    STANDBYSERVICE_LOGD("condition is %{public}u", GetCondition());
    if (GetCondition() == ConditionType::NIGHT_STANDBY) {
        res = DiffToFixedClock(curSecTimeStamp, DAY_ENTRANCE_HOUR, DAY_ENTRANCE_MIN, timeDiff);
    } else {
        res = DiffToFixedClock(curSecTimeStamp, NIGHT_ENTRANCE_HOUR, NIGHT_ENTRANCE_MIN, timeDiff);
    }
    return res;
}

bool TimeProvider::DiffToFixedClock(int64_t curTimeStamp, int32_t tmHour, int32_t tmMin, int64_t& timeDiff)
{
    struct tm curUTCTime {};
    auto res = gmtime_r(&curTimeStamp, &curUTCTime);
    if (res == nullptr) {
        STANDBYSERVICE_LOGE("gmtime_r failed, can not get valid utc time");
        return false;
    }
    STANDBYSERVICE_LOGD("current utc time info: %{public}02d:%{public}02d:%{public}02d", curUTCTime.tm_hour,
        curUTCTime.tm_min, curUTCTime.tm_sec);
    struct tm targetUTCTime = curUTCTime;
    targetUTCTime.tm_hour = tmHour;
    targetUTCTime.tm_min = tmMin;
    targetUTCTime.tm_sec = 0;
    STANDBYSERVICE_LOGD("target utc time info: %{public}02d:%{public}02d:%{public}02d", targetUTCTime.tm_hour,
        targetUTCTime.tm_min, targetUTCTime.tm_sec);
    timeDiff = std::mktime(&targetUTCTime) - curTimeStamp;
    if (timeDiff < 0) {
        timeDiff += SEC_PER_DAY;
    }
    timeDiff = timeDiff * MSEC_PER_SEC;
    return true;
}

int64_t TimeProvider::GetNapTimeOut()
{
    int64_t curSecTimeStamp = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs() / MSEC_PER_SEC;
    int32_t napTimeOut = TWENTY_MIN_ENTRANCE_MIN;
    struct tm curLocalTime {};
#ifdef STANDBY_POWER_MANAGER_ENABLE
    if (IsPowerSaveMode()) {
        return SAVE_MODE_ENTRANCE_MIN * MSEC_PER_MIN;
    }
#endif
    if (!ConvertTimeStampToLocalTime(curSecTimeStamp, curLocalTime)) {
        return napTimeOut * MSEC_PER_MIN;
    }
    if (curLocalTime.tm_hour == NIGHT_TWENTY_TWO_CLOCK) {
        napTimeOut = QUARTER_MIN_ENTRANCE_MIN;
    } else if (curLocalTime.tm_hour == NIGHT_TWENTY_THREE_CLOCK && curLocalTime.tm_min < THREE_QUARTERS) {
        napTimeOut = QUARTER_MIN_ENTRANCE_MIN;
    } else if (curLocalTime.tm_hour >= NIGHT_TWENTY_THREE_CLOCK || curLocalTime.tm_hour < NIGHT_FIVE_CLOCK ||
        (curLocalTime.tm_hour == NIGHT_FIVE_CLOCK && curLocalTime.tm_min < THREE_QUARTERS)) {
        napTimeOut = TEN_MIN_ENTRANCE_MIN;
    }
    return napTimeOut * MSEC_PER_MIN;
}

#ifdef STANDBY_POWER_MANAGER_ENABLE
bool TimeProvider::IsPowerSaveMode()
{
    PowerMgr::PowerMode mode = PowerMgr::PowerMgrClient::GetInstance().GetDeviceMode();
    return (mode == PowerMgr::PowerMode::POWER_SAVE_MODE || mode == PowerMgr::PowerMode::EXTREME_POWER_SAVE_MODE);
}
#endif

int32_t TimeProvider::GetRandomDelay(int32_t low, int32_t high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS

