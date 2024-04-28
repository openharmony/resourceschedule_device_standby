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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_STANDBY_MESSAGE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_STANDBY_MESSAGE_H

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <optional>

#include "want.h"

#include "common_constant.h"

namespace OHOS {
namespace DevStandbyMgr {
struct StandbyMessageType {
    enum : uint32_t {
        COMMON_EVENT = 1,
        STATE_TRANSIT,
        PHASE_TRANSIT,
        RES_CTRL_CONDITION_CHANGED,  // day and night switch
        ALLOW_LIST_CHANGED,
        SCREEN_OFF_HALF_HOUR,
        BG_TASK_STATUS_CHANGE,  // application apply or unapply background task, start or stop work scheduler
        SYS_ABILITY_STATUS_CHANGED,  // system ability is added or removed
        PROCESS_STATE_CHANGED,  // process is created or died
        DEVICE_STATE_CHANGED,  // process is created or died
        USER_SLEEP_STATE_CHANGED, // user is sleep or not
        DEVICE_NET_IDLE_POLICY_TRANSIT, // netlimit or not
        BG_EFFICIENCY_RESOURCE_APPLY // application apply or unapply efficiency resources
    };
};

struct StandbyMessage {
    StandbyMessage() = default;
    explicit StandbyMessage(uint32_t eventId): eventId_(eventId) {}
    StandbyMessage(uint32_t eventId, const std::string& action): eventId_(eventId), action_(action) {}

    uint32_t eventId_ {0};
    std::string action_ {""};
    std::optional<AAFwk::Want> want_ {};
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_PLUGINS_EXT_INCLUDE_STANDBY_MESSAGE_H
