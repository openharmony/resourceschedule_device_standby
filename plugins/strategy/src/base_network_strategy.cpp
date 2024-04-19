/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "base_network_strategy.h"
#include <algorithm>
#include "system_ability_definition.h"
#ifdef STANDBY_RSS_WORK_SCHEDULER_ENABLE
#include "workscheduler_srv_client.h"
#endif

#include "standby_service_log.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_helper.h"
#endif
#ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "net_policy_client.h"
#endif
#include "app_mgr_helper.h"
#include "standby_service.h"
#include "allow_type.h"
#include "standby_state.h"
#include "bundle_manager_helper.h"
#include "standby_config_manager.h"
#include "time_provider.h"
#include "standby_service_impl.h"
#include "common_constant.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
const std::map<std::string, uint8_t> BGTASK_EXEMPTION_FLAG_MAP {
    {CONTINUOUS_TASK, ExemptionTypeFlag::CONTINUOUS_TASK},
    {TRANSIENT_TASK, ExemptionTypeFlag::TRANSIENT_TASK},
    {WORK_SCHEDULER, ExemptionTypeFlag::WORK_SCHEDULER},
};
}

bool BaseNetworkStrategy::isFirewallEnabled_ = false;
std::unordered_map<std::int32_t, NetLimtedAppInfo> BaseNetworkStrategy::netLimitedAppInfo_;
static std::mutex mutex_;

void BaseNetworkStrategy::HandleEvent(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("BaseNetworkStrategy revceived message %{public}u, action: %{public}s",
        message.eventId_, message.action_.c_str());
    switch (message.eventId_) {
        case StandbyMessageType::ALLOW_LIST_CHANGED:
            UpdateExemptionList(message);
            break;
        case StandbyMessageType::BG_TASK_STATUS_CHANGE:
            UpdateBgTaskAppStatus(message);
            break;
        case StandbyMessageType::PROCESS_STATE_CHANGED:
            HandleProcessStatusChanged(message);
            break;
        case StandbyMessageType::SYS_ABILITY_STATUS_CHANGED:
            ResetFirewallStatus(message);
            break;
        default:
            break;
    }
}

ErrCode BaseNetworkStrategy::OnCreated()
{
    // when initialized, stop net limit mode in case of unexpected process restart
    ResetFirewallAllowList();
    isFirewallEnabled_ = false;
    isIdleMaintence_ = false;
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::OnDestroy()
{
    ResetFirewallAllowList();
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::UpdateExemptionList(const StandbyMessage& message)
{
    uint32_t allowType = static_cast<uint32_t>(message.want_->GetIntParam("allowType", 0));
    if ((allowType & AllowType::NETWORK) == 0) {
        STANDBYSERVICE_LOGD("allowType is not network, currentType is %{public}d", allowType);
        return ERR_STANDBY_STRATEGY_NOT_MATCH;
    }
    if (!isFirewallEnabled_) {
        STANDBYSERVICE_LOGD("current state is not sleep or maintenance, ignore exemption");
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }
    // start update exemption flag
    std::string processName = message.want_->GetStringParam("name");
    bool added = message.want_->GetBoolParam("added", false);
    int32_t uid = message.want_->GetIntParam("uid", -1);
    STANDBYSERVICE_LOGI("updatee exemption list, %{public}s apply exemption, added is %{public}d",
        processName.c_str(), added);
    if (added) {
        AddExemptionFlag(uid, processName, ExemptionTypeFlag::EXEMPTION);
    } else {
        RemoveExemptionFlag(uid, ExemptionTypeFlag::EXEMPTION);
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::UpdateFirewallAllowList()
{
    ResetFirewallAllowList();
    netLimitedAppInfo_.clear();
    if (InitNetLimitedAppInfo() != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    SetFirewallStatus(true);
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::EnableNetworkFirewall(const StandbyMessage& message)
{
    if (isFirewallEnabled_) {
        STANDBYSERVICE_LOGD("now is doze, do not need start net limit mode, repeat process");
        return ERR_STANDBY_STRATEGY_STATE_REPEAT;
    }
    STANDBYSERVICE_LOGD("start net limit mode");
    // if enter sleep state and app_res_deep phase, start net limit mode.
    if (auto ret = EnableNetworkFirewallInner(); ret != ERR_OK) {
        return ret;
    }
    isFirewallEnabled_ = true;
    isIdleMaintence_ = false;
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::EnableNetworkFirewallInner()
{
    netLimitedAppInfo_.clear();
    if (InitNetLimitedAppInfo() != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    SetFirewallStatus(true);
    return ERR_OK;
}

// get app info, add exemption according to the status of app.
ErrCode BaseNetworkStrategy::InitNetLimitedAppInfo()
{
    if (GetAllRunningAppInfo() != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    std::vector<AppExecFwk::ApplicationInfo> applicationInfos {};
    if (!BundleManagerHelper::GetInstance()->GetApplicationInfos(
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        AppExecFwk::Constants::ALL_USERID, applicationInfos)) {
        STANDBYSERVICE_LOGW("failed to get all applicationInfos");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGD("succeed GetApplicationInfos, size is %{public}d",
        static_cast<int32_t>(applicationInfos.size()));
    for (const auto& info : applicationInfos) {
        if (netLimitedAppInfo_.find(info.uid) == netLimitedAppInfo_.end()) {
            continue;
        }
        if (info.isSystemApp) {
            netLimitedAppInfo_[info.uid].appExemptionFlag_ |= ExemptionTypeFlag::UNRESTRICTED;
        }
    }

    if (GetForegroundApplications() !=ERR_OK || GetBackgroundTaskApp() != ERR_OK ||
        GetWorkSchedulerTask() != ERR_OK || GetExemptionConfig() != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::GetAllRunningAppInfo()
{
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos {};
    if (!AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos)) {
        STANDBYSERVICE_LOGE("connect to app manager service failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGI("current running processes size %{public}d", static_cast<int32_t>(allAppProcessInfos.size()));
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &info : allAppProcessInfos) {
        netLimitedAppInfo_.emplace(info.uid_, NetLimtedAppInfo {info.processName_});
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::GetForegroundApplications()
{
    std::vector<AppExecFwk::AppStateData> fgApps {};
    if (!AppMgrHelper::GetInstance()->GetForegroundApplications(fgApps)) {
        STANDBYSERVICE_LOGW("get foreground app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    for (const auto& appInfo : fgApps) {
        AddExemptionFlagByUid(appInfo.uid, ExemptionTypeFlag::FOREGROUND_APP);
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::GetBackgroundTaskApp()
{
    #ifdef ENABLE_BACKGROUND_TASK_MGR
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> continuousTaskList;
    if (!BackgroundTaskHelper::GetInstance()->GetContinuousTaskApps(continuousTaskList)) {
        STANDBYSERVICE_LOGW("get continuous task app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGD("succeed GetContinuousTaskApps, size is %{public}d",
        static_cast<int32_t>(continuousTaskList.size()));
    std::vector<std::shared_ptr<TransientTaskAppInfo>> transientTaskList;
    if (!BackgroundTaskHelper::GetInstance()->GetTransientTaskApps(transientTaskList)) {
        STANDBYSERVICE_LOGW("get transient task app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGD("succeed GetTransientTaskApps, size is %{public}d",
        static_cast<int32_t>(transientTaskList.size()));
    condition_ = TimeProvider::GetCondition();
    for (const auto& task : continuousTaskList) {
        if (auto infoIter = netLimitedAppInfo_.find(task->GetCreatorUid()); infoIter == netLimitedAppInfo_.end()) {
            continue;
        } else if (condition_ == ConditionType::DAY_STANDBY ||
            (nightExemptionTaskType_ & (1<< task->GetTypeId())) != 0) {
            infoIter->second.appExemptionFlag_ |= ExemptionTypeFlag::CONTINUOUS_TASK;
        }
    }
    for (const auto& task : transientTaskList) {
        AddExemptionFlagByUid(task->GetUid(), ExemptionTypeFlag::TRANSIENT_TASK);
    }
    #endif
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::GetWorkSchedulerTask()
{
    #ifdef STANDBY_RSS_WORK_SCHEDULER_ENABLE
    std::list<std::shared_ptr<WorkScheduler::WorkInfo>> workInfos;
    if (WorkScheduler::WorkSchedulerSrvClient::GetInstance().GetAllRunningWorks(workInfos) != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGD("GetWorkSchedulerTask succeed, size is %{public}d", static_cast<int32_t>(workInfos.size()));
    for (const auto& task : workInfos) {
        AddExemptionFlagByUid(task->GetUid(), ExemptionTypeFlag::WORK_SCHEDULER);
    }
    #endif
    return ERR_OK;
}

void BaseNetworkStrategy::AddExemptionFlagByUid(int32_t uid, uint8_t flag)
{
    if (auto iter = netLimitedAppInfo_.find(uid); iter != netLimitedAppInfo_.end()) {
        iter->second.appExemptionFlag_ |= flag;
    }
}

ErrCode BaseNetworkStrategy::GetExemptionConfig()
{
    // if app in exemption list, add exemption flag
    std::vector<AllowInfo> allowInfoList {};
    StandbyServiceImpl::GetInstance()->GetAllowListInner(AllowType::NETWORK, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    std::set<std::string> allowNameList {};
    for (const auto& info : allowInfoList) {
        allowNameList.emplace(info.GetName());
    }
    for (auto& [key, value] : netLimitedAppInfo_) {
        if (allowNameList.find(value.name_) == allowNameList.end()) {
            continue;
        }
        value.appExemptionFlag_ |= ExemptionTypeFlag::EXEMPTION;
    }
    // if app in restricted list, add retricted flag
    std::set<std::string> restrictNameList {};
    StandbyServiceImpl::GetInstance()->GetEligiableRestrictSet(AllowType::NETWORK, "NETWORK",
        ReasonCodeEnum::REASON_APP_API, restrictNameList);
    for (auto& [key, value] : netLimitedAppInfo_) {
        if (restrictNameList.find(value.name_) == restrictNameList.end()) {
            continue;
        }
        value.appExemptionFlag_ |= ExemptionTypeFlag::RESTRICTED;
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::GetExemptionConfigForApp(NetLimtedAppInfo& appInfo, const std::string& bundleName)
{
    // if app in exemption list, add exemption flag
    std::vector<AllowInfo> allowInfoList {};
    StandbyServiceImpl::GetInstance()->GetAllowListInner(AllowType::NETWORK, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    std::set<std::string> allowNameList {};
    for (const auto& info : allowInfoList) {
        if (info.GetName() != bundleName) {
            continue;
        }
        appInfo.appExemptionFlag_ |= ExemptionTypeFlag::EXEMPTION;
    }

    // if app in restricted list, add retricted flag
    std::set<std::string> restrictNameList {};
    StandbyServiceImpl::GetInstance()->GetEligiableRestrictSet(AllowType::NETWORK, "NETWORK",
        ReasonCodeEnum::REASON_APP_API, restrictNameList);
    if (restrictNameList.find(bundleName) != restrictNameList.end()) {
        appInfo.appExemptionFlag_ |= ExemptionTypeFlag::RESTRICTED;
    }
    return ERR_OK;
}

void BaseNetworkStrategy::SetNetAllowApps(bool isAllow)
{
    std::vector<uint32_t> uids;
    for (const auto& [key, value] : netLimitedAppInfo_) {
        if (!ExemptionTypeFlag::IsExempted(value.appExemptionFlag_)) {
            continue;
        }
        if ((condition_ == ConditionType::NIGHT_STANDBY) &&
            (!ExemptionTypeFlag::IsExempted(value.appExemptionFlag_ & (~ExemptionTypeFlag::UNRESTRICTED)))) {
            continue;
        }
        uids.emplace_back(key);
        STANDBYSERVICE_LOGD("uid: %{public}d, name: %{public}s, isAllow: %{public}d",
            key, value.name_.c_str(), isAllow);
    }
    STANDBYSERVICE_LOGD("all application size: %{public}d, network allow: %{public}d",
        static_cast<int32_t>(netLimitedAppInfo_.size()), static_cast<int32_t>(uids.size()));
    SetFirewallAllowedList(uids, isAllow);
}

ErrCode BaseNetworkStrategy::DisableNetworkFirewall(const StandbyMessage& message)
{
    if (!isFirewallEnabled_) {
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }
    uint32_t preState = static_cast<uint32_t>(message.want_->GetIntParam(PREVIOUS_STATE, 0));
    uint32_t curState = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_STATE, 0));
    if ((curState == StandbyState::MAINTENANCE) && (preState == StandbyState::SLEEP)) {
        // restart net limit mode
        SetFirewallStatus(false);
        isIdleMaintence_ = true;
        isFirewallEnabled_ = false;
    } else if ((curState == StandbyState::SLEEP) && (preState == StandbyState::MAINTENANCE)) {
        isIdleMaintence_ = false;
        // stop net limit mode
        SetFirewallStatus(true);
        isFirewallEnabled_ = true;
    } else if (preState == StandbyState::SLEEP || preState == StandbyState::MAINTENANCE) {
        DisableNetworkFirewallInner();
        isIdleMaintence_ = false;
        isFirewallEnabled_ = false;
    }
    return ERR_OK;
}

ErrCode BaseNetworkStrategy::DisableNetworkFirewallInner()
{
    ResetFirewallAllowList();
    netLimitedAppInfo_.clear();
    return ERR_OK;
}

int32_t BaseNetworkStrategy::HandleDeviceIdlePolicy(bool enableFirewall)
{
    int32_t ret = NETMANAGER_SUCCESS;
    #ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
    ret = DelayedSingleton<NetManagerStandard::NetPolicyClient>::GetInstance()->
        SetDeviceIdlePolicy(enableFirewall);
    if (ret == NETMANAGER_SUCCESS || ret == NETMANAGER_ERR_STATUS_EXIST) {
        StandbyMessage standbyMessage {StandbyMessageType::DEVICE_NET_IDLE_POLICY_TRANSIT};
        standbyMessage.want_ = AAFwk::Want{};
        standbyMessage.want_->SetParam(NET_IDLE_POLICY_STATUS, enableFirewall);
        StandbyServiceImpl::GetInstance()->DispatchEvent(standbyMessage);
    }
    #endif
    return ret;
}

ErrCode BaseNetworkStrategy::UpdateBgTaskAppStatus(const StandbyMessage& message)
{
    if (!isFirewallEnabled_) {
        STANDBYSERVICE_LOGD("current state is not sleep or maintenance, ignore exemption");
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }

    std::string type = message.want_->GetStringParam(BG_TASK_TYPE);
    bool started = message.want_->GetBoolParam(BG_TASK_STATUS, false);
    int32_t uid = message.want_->GetIntParam(BG_TASK_UID, 0);
    std::string bundleName = message.want_->GetStringParam(BG_TASK_BUNDLE_NAME);
    if (BGTASK_EXEMPTION_FLAG_MAP.find(type) == BGTASK_EXEMPTION_FLAG_MAP.end()) {
        return ERR_STANDBY_KEY_INFO_NOT_MATCH;
    }
    if (bundleName.empty()) {
        bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    }
    if (started) {
        AddExemptionFlag(static_cast<uint32_t>(uid), bundleName, BGTASK_EXEMPTION_FLAG_MAP.at(type));
    } else {
        RemoveExemptionFlag(static_cast<uint32_t>(uid), BGTASK_EXEMPTION_FLAG_MAP.at(type));
    }
    return ERR_OK;
}

void BaseNetworkStrategy::HandleProcessStatusChanged(const StandbyMessage& message)
{
    if (!isFirewallEnabled_) {
        STANDBYSERVICE_LOGD("current state is not sleep or maintenance, ignore state of process");
        return;
    }
    int32_t uid = message.want_->GetIntParam("uid", -1);
    std::string bundleName = message.want_->GetStringParam("name");
    bool isCreated = message.want_->GetBoolParam("isCreated", false);
    if (isCreated) {
        GetAndCreateAppInfo(uid, bundleName);
        auto iter = netLimitedAppInfo_.find(uid);
        if (ExemptionTypeFlag::IsExempted(iter->second.appExemptionFlag_)) {
            SetFirewallAllowedList({uid}, isCreated);
        }
    } else {
        bool isRunning {false};
        if (AppMgrHelper::GetInstance()->GetAppRunningStateByBundleName(bundleName, isRunning) && !isRunning) {
            std::lock_guard<std::mutex> lock(mutex_);
            netLimitedAppInfo_.erase(uid);
            SetFirewallAllowedList({uid}, isCreated);
        }
    }
}

void BaseNetworkStrategy::AddExemptionFlag(uint32_t uid, const std::string& bundleName, uint8_t flag)
{
    if (!isFirewallEnabled_) {
        return;
    }
    STANDBYSERVICE_LOGD("AddExemptionFlag uid is %{public}u, flag is %{public}d", uid, flag);
    GetAndCreateAppInfo(uid, bundleName);
    auto iter = netLimitedAppInfo_.find(uid);
    if (iter == netLimitedAppInfo_.end()) {
        netLimitedAppInfo_[uid] = NetLimtedAppInfo {bundleName};
    }
    auto lastAppExemptionFlag = iter->second.appExemptionFlag_;
    iter->second.appExemptionFlag_ |= flag;
    if (GetExemptedFlag(lastAppExemptionFlag, iter->second.appExemptionFlag_)) {
        SetFirewallAllowedList({iter->first}, true);
    }
    iter->second.appExemptionFlag_ |= flag;
}

void BaseNetworkStrategy::RemoveExemptionFlag(uint32_t uid, uint8_t flag)
{
    if (!isFirewallEnabled_) {
        return;
    }
    auto iter = netLimitedAppInfo_.find(uid);
    if (iter == netLimitedAppInfo_.end()) {
        return;
    }

    STANDBYSERVICE_LOGD("RemoveExemptionFlag uid is flag is %{public}d, flag is %{public}d", uid, flag);
    auto lastAppExemptionFlag = iter->second.appExemptionFlag_;
    iter->second.appExemptionFlag_ &= (~flag);
    if (GetExemptedFlag(iter->second.appExemptionFlag_, lastAppExemptionFlag)) {
        SetFirewallAllowedList({iter->first}, false);
    }
}

bool BaseNetworkStrategy::GetExemptedFlag(uint8_t appNoExemptionFlag, uint8_t appExemptionFlag)
{
    condition_ = TimeProvider::GetCondition();
    if ((condition_ == ConditionType::NIGHT_STANDBY) &&
        (!ExemptionTypeFlag::IsExempted(appNoExemptionFlag & (~ExemptionTypeFlag::UNRESTRICTED))) &&
        ExemptionTypeFlag::IsExempted(appExemptionFlag)) {
        return true;
    } else if (!ExemptionTypeFlag::IsExempted(appNoExemptionFlag) &&
        ExemptionTypeFlag::IsExempted(appExemptionFlag)) {
        return true;
    }
    return false;
}

void BaseNetworkStrategy::ResetFirewallAllowList()
{
    #ifdef STANDBY_COMMUNICATION_NETMANAGER_BASE_ENABLE
    STANDBYSERVICE_LOGI("start reset firewall allow list");
    std::vector<uint32_t> uids;
    if (DelayedSingleton<NetManagerStandard::NetPolicyClient>::GetInstance()->
        GetDeviceIdleTrustlist(uids) != NETMANAGER_SUCCESS) {
        STANDBYSERVICE_LOGE("get deviceIdle netLimited list is failed");
        return;
    }
    int32_t ret = HandleDeviceIdlePolicy(false);
    if (ret != NETMANAGER_SUCCESS && ret != NETMANAGER_ERR_STATUS_EXIST) {
        STANDBYSERVICE_LOGE("handle device idle policy netLimited is false");
        return;
    }
    if (DelayedSingleton<NetManagerStandard::NetPolicyClient>::GetInstance()->
        SetDeviceIdleTrustlist(uids, false) != NETMANAGER_SUCCESS) {
        STANDBYSERVICE_LOGE("SetFirewallAllowedList failed");
    }
    #endif
}

ErrCode BaseNetworkStrategy::SetFirewallStatus(bool enableFirewall)
{
    int32_t ret = HandleDeviceIdlePolicy(enableFirewall);
    STANDBYSERVICE_LOGD("set status of powersaving firewall: %{public}d , res: %{public}d",
        enableFirewall, ret);
    if (ret == NETMANAGER_SUCCESS || (!enableFirewall && ret == NETMANAGER_ERR_STATUS_EXIST)) {
        SetNetAllowApps(enableFirewall);
        return ERR_OK;
    } else {
        STANDBYSERVICE_LOGE("Failed to enable or disable powersaving firewall");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
}

// when app is created, add app info to cache
void BaseNetworkStrategy::GetAndCreateAppInfo(uint32_t uid, const std::string& bundleName)
{
    auto iter = netLimitedAppInfo_.find(uid);
    if (iter != netLimitedAppInfo_.end()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::tie(iter, std::ignore) = netLimitedAppInfo_.emplace(uid, NetLimtedAppInfo {bundleName});

    bool isSystemApp {false};
    if (BundleManagerHelper::GetInstance()->CheckIsSystemAppByUid(uid, isSystemApp) && isSystemApp) {
        iter->second.appExemptionFlag_ |= ExemptionTypeFlag::UNRESTRICTED;
    }
    GetExemptionConfigForApp(iter->second, bundleName);
}

// when bgtask or work_scheduler service crash, reset relative flag
void BaseNetworkStrategy::ResetFirewallStatus(const StandbyMessage& message)
{
    if (!isFirewallEnabled_ || isIdleMaintence_) {
        return;
    }
    bool isAdded = message.want_->GetBoolParam(SA_STATUS, false);
    if (isAdded) {
        return;
    }
    int32_t saId = message.want_->GetIntParam(SA_ID, 0);
    if (saId != WORK_SCHEDULE_SERVICE_ID && saId != BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        return;
    }
    const uint8_t bgTaskFlag = (ExemptionTypeFlag::TRANSIENT_TASK | ExemptionTypeFlag::WORK_SCHEDULER);
    for (const auto& [key, value] : netLimitedAppInfo_) {
        if ((value.appExemptionFlag_ & bgTaskFlag) == 0) {
            continue;
        }
        RemoveExemptionFlag(key, (value.appExemptionFlag_ & bgTaskFlag));
    }
    return;
}

void BaseNetworkStrategy::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    result.append("Network Strategy:\n").append("isFirewallEnabled: " + std::to_string(isFirewallEnabled_))
        .append(" isIdleMaintence: " + std::to_string(isIdleMaintence_)).append("\n");
    result.append("limited app info: \n");
    for (const auto& [key, value] : netLimitedAppInfo_) {
        result.append("uid: ").append(std::to_string(key)).append(" name: ").append(value.name_).append(" uid: ")
            .append(std::to_string(key)).append(" exemption flag: ")
            .append(std::to_string(value.appExemptionFlag_)).append("\n");
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
