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

#include "network_strategy.h"

#ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "net_policy_client.h"
#endif

#include "standby_state.h"
#include "time_provider.h"
#include "istandby_service.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
ErrCode NetworkStrategy::OnCreated()
{
    STANDBYSERVICE_LOGI("NetworkStrategy is now OnCreated");
    condition_ = TimeProvider::GetCondition();
    ResetFirewallAllowList();
    return ERR_OK;
}

ErrCode NetworkStrategy::OnDestroy()
{
    STANDBYSERVICE_LOGI("NetworkStrategy is now OnDestroy");
    ResetFirewallAllowList();
    return ERR_OK;
}

void NetworkStrategy::HandleEvent(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("enter NetworkStrategy HandleEvent, eventId is %{public}d", message.eventId_);
    switch (message.eventId_) {
        case StandbyMessageType::ALLOW_LIST_CHANGED:
            UpdateAllowedList(message);
            break;
        case StandbyMessageType::RES_CTRL_CONDITION_CHANGED:
            UpdateNetResourceConfig(message);
            break;
        case StandbyMessageType::PHASE_TRANSIT:
            StartNetLimit(message);
            break;
        case StandbyMessageType::STATE_TRANSIT:
            StopNetLimit(message);
            break;
        case StandbyMessageType::BG_TASK_STATUS_CHANGE:
            UpdateBgTaskAppStatus(message);
            break;
        case StandbyMessageType::PROCESS_STATE_CHANGED:
            HandleProcessStatusChanged(message);
            break;
        default:
            break;
    }
}

void NetworkStrategy::UpdateAllowedList(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("enter NetworkStrategy UpdateAllowedList, eventId is %{public}d", message.eventId_);
    UpdateExemptionList(message);
}

void NetworkStrategy::UpdateNetResourceConfig(const StandbyMessage& message)
{
    condition_ = static_cast<uint32_t>(message.want_->GetIntParam(RES_CTRL_CONDITION, 0));
    STANDBYSERVICE_LOGD("enter NetworkStrategy HandleEvent, current condition is %{public}u", condition_);
    UpdateFirewallAllowList();
}

void NetworkStrategy::StartNetLimit(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("enter NetworkStrategy StartNetLimit, eventId is %{public}d", message.eventId_);
    uint32_t current_phase = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_PHASE, 0));
    uint32_t current_state = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_STATE, 0));
    if ((current_state != StandbyState::SLEEP) || (current_phase != SleepStatePhase::APP_RES_DEEP)) {
        STANDBYSERVICE_LOGD("current state is not SLEEP or current phase is not APP_RES_DEEP!");
        return;
    }
    EnableNetworkFirewall(message);
}

void NetworkStrategy::StopNetLimit(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("enter NetworkStrategy StopNetLimit, eventId is %{public}d", message.eventId_);
    DisableNetworkFirewall(message);
}

void NetworkStrategy::SetFirewallAllowedList(const std::vector<uint32_t>& uids, bool isAdded)
{
    STANDBYSERVICE_LOGD("SetFireWallAllowedList, uids size %{public}d, isAdded is %{public}d",
        static_cast<int32_t>(uids.size()), isAdded);
    if (uids.empty()) {
        STANDBYSERVICE_LOGD("allow list is empty");
        return;
    }
    if (!isAdded && isIdleMaintence_) {
        STANDBYSERVICE_LOGI("current is idle maintenance, do not need remove allow list");
        return;
    }
    #ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
    if (auto ret = DelayedSingleton<NetManagerStandard::NetPolicyClient>::GetInstance()->
        SetDeviceIdleTrustlist(uids, isAdded); ret != 0) {
        STANDBYSERVICE_LOGW("failed to SetFireWallAllowedList, err code is %{public}d", ret);
        return;
    }
    #endif
}

void NetworkStrategy::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr[DUMP_FIRST_PARAM] == DUMP_DETAIL_INFO &&
        argsInStr[DUMP_SECOND_PARAM] == DUMP_STRATGY_DETAIL) {
        result.append("=================DeviceIdle=======================\n");
        BaseNetworkStrategy::ShellDump(argsInStr, result);
    }
}
} // namespace DevStandbyMgr
} // namespace OHOS
