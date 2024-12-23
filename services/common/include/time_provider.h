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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_INCLUDE_TIME_PROVIDER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_INCLUDE_TIME_PROVIDER_H

#include <ctime>
#include <cstdint>

namespace OHOS {
namespace DevStandbyMgr {
enum ClockType {
    CLOCK_TYPE_REALTIME = CLOCK_REALTIME,
    CLOCK_TYPE_MONOTONIC = CLOCK_MONOTONIC,
};

enum TimeConstant : int64_t {
    NSEC_PER_MIN = 60000000000LL,
    NSEC_PER_SEC = 1000000000LL,
    NSEC_PER_MSEC = 1000000LL,
    NSEC_PER_USEC = 1000LL,
    USEC_PER_SEC = 1000000LL,
    USEC_PER_MSEC = 1000LL,
    MSEC_PER_DAY = 86400000LL,
    MSEC_PER_DAWN = 21600000LL,
    MSEC_PER_HOUR = 3600000LL,
    MSEC_PER_MIN = 60000LL,
    MSEC_PER_SEC = 1000LL,
    SEC_PER_MIN = 60LL,
    SEC_PER_HOUR = 3600LL,
    SEC_PER_DAY = 86400LL,
    MIN_PER_HOUR = 60LL,
    HOUR_PER_DAY = 24LL,
};

class TimeProvider {
public:
    static bool ConvertTimeStampToLocalTime(int64_t curTimeStamp, struct tm& curLocalTime);
    static uint32_t GetCondition(int64_t afterNextSeconds = 0);
    static int64_t GetNapTimeOut();
    static bool TimeDiffToDayNightSwitch(int64_t& timeDiff);
    static int32_t GetRandomDelay(int32_t low, int32_t high);
    static bool DiffToFixedClock(int64_t curTimeStamp, int32_t tmHour, int32_t tmMin, int64_t& timeDiff);
    static int32_t GetCurrentDate();

#ifdef STANDBY_POWER_MANAGER_ENABLE
private:
    static bool IsPowerSaveMode();
#endif
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_INCLUDE_TIME_PROVIDER_H
