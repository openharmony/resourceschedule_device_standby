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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ALLOW_RECORD_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ALLOW_RECORD_H

#include <list>
#include <string>
#include <cstdint>
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DevStandbyMgr {
struct AllowTime {
    AllowTime() = default;
    AllowTime(uint32_t allowTypeIndex, int64_t endTime, const std::string& reason)
        : allowTypeIndex_(allowTypeIndex), endTime_(endTime), reason_(reason) {}
    uint32_t allowTypeIndex_;
    int64_t endTime_ {0};
    std::string reason_ {""};
};

struct AllowRecord {
    AllowRecord() = default;
    AllowRecord(int32_t uid, int32_t pid, const std::string& name, uint32_t allowType)
        : uid_(uid), pid_(pid), name_(name), allowType_(allowType) {}
    nlohmann::json ParseToJson();
    bool setAllowTime(const nlohmann::json& persistTime);
    bool setAllowRecordField(const nlohmann::json& value);
    bool ParseFromJson(const nlohmann::json& value);

    int32_t uid_ {-1};
    int32_t pid_ {-1};
    std::string name_ {""};
    uint32_t allowType_ {0};
    std::list<AllowTime> allowTimeList_ {};
    uint32_t reasonCode_ {0};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_ALLOW_RECORD_H