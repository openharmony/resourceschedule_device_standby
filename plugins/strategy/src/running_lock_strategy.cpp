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

#include "running_lock_strategy.h"
#include <algorithm>
#include "standby_service_log.h"
#include "system_ability_definition.h"

#include "ability_manager_helper.h"
#ifdef ENABLE_BACKGROUND_TASK_MGR
#include "background_task_helper.h"
#endif
#include "app_mgr_helper.h"
#include "standby_service.h"
#ifdef STANDBY_POWER_MANAGER_ENABLE
#include "power_mgr_client.h"
#endif
#ifdef STANDBY_RSS_WORK_SCHEDULER_ENABLE
#include "workscheduler_srv_client.h"
#endif
#include "allow_type.h"
#include "standby_state.h"
#include "bundle_manager_helper.h"
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

void RunningLockStrategy::HandleEvent(const StandbyMessage& message)
{
    STANDBYSERVICE_LOGD("RunningLockStrategy revceived message %{public}u, action: %{public}s",
        message.eventId_, message.action_.c_str());
    switch (message.eventId_) {
        case StandbyMessageType::ALLOW_LIST_CHANGED:
            UpdateExemptionList(message);
            break;
        case StandbyMessageType::RES_CTRL_CONDITION_CHANGED:
            UpdateResourceConfig(message);
            break;
        case StandbyMessageType::PHASE_TRANSIT:
            StartProxy(message);
            break;
        case StandbyMessageType::STATE_TRANSIT:
            StopProxy(message);
            break;
        case StandbyMessageType::BG_TASK_STATUS_CHANGE:
            UpdateBgTaskAppStatus(message);
            break;
        case StandbyMessageType::PROCESS_STATE_CHANGED:
            HandleProcessStatusChanged(message);
            break;
        case StandbyMessageType::SYS_ABILITY_STATUS_CHANGED:
            ResetProxyStatus(message);
            break;
        default:
            break;
    }
}

ErrCode RunningLockStrategy::OnCreated()
{
    #ifdef STANDBY_POWER_MANAGER_ENABLE
    PowerMgr::PowerMgrClient::GetInstance().ResetRunningLocks();
    #endif
    return ERR_OK;
}

ErrCode RunningLockStrategy::OnDestroy()
{
    if (isProxied_ && !isIdleMaintence_) {
        ProxyAppAndProcess(false);
    }
    isProxied_ = false;
    isIdleMaintence_ = false;
    return ERR_OK;
}

ErrCode RunningLockStrategy::UpdateExemptionList(const StandbyMessage& message)
{
    uint32_t allowType = static_cast<uint32_t>(message.want_->GetIntParam("allowType", 0));
    if ((allowType & AllowType::RUNNING_LOCK) == 0) {
        STANDBYSERVICE_LOGD("allowType is not running lock, currentType is %{public}d", allowType);
        return ERR_STANDBY_STRATEGY_NOT_MATCH;
    }
    if (!isProxied_) {
        STANDBYSERVICE_LOGD("current state is not sleep or maintenance, ignore exemption");
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }

    // according to message, add flag or remove flag
    STANDBYSERVICE_LOGI("RunningLockStrategy start update allow list");
    std::string processName = message.want_->GetStringParam("name");
    bool added = message.want_->GetBoolParam("added", false);
    int32_t uid = message.want_->GetIntParam("uid", -1);
    STANDBYSERVICE_LOGD("%{public}s apply allow, added is %{public}d", processName.c_str(), added);
    if (added) {
        AddExemptionFlag(uid, processName, ExemptionTypeFlag::EXEMPTION);
    } else {
        RemoveExemptionFlag(uid, processName, ExemptionTypeFlag::EXEMPTION);
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::UpdateResourceConfig(const StandbyMessage& message)
{
    StopProxyInner();
    StartProxyInner();
    return ERR_OK;
}

ErrCode RunningLockStrategy::StartProxy(const StandbyMessage& message)
{
    if (isProxied_) {
        STANDBYSERVICE_LOGD("now is proxied, do not need StartProxy, repeat process");
        return ERR_STANDBY_STRATEGY_STATE_REPEAT;
    }
    uint32_t curPhase = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_PHASE, 0));
    uint32_t curState = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_STATE, 0));
    if ((curState != StandbyState::SLEEP) || (curPhase != SleepStatePhase::APP_RES_DEEP)) {
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }
    STANDBYSERVICE_LOGD("start proxy running lock, current state is %{public}d, current phase is %{public}d",
        curPhase, curPhase);
    // if enter sleep state and app_res_deep phase, start proxy running lock of applications.
    if (auto ret = StartProxyInner(); ret != ERR_OK) {
        return ret;
    }
    isProxied_ = true;
    isIdleMaintence_ = false;
    return ERR_OK;
}

ErrCode RunningLockStrategy::StartProxyInner()
{
    ClearProxyRecord();
    if (InitProxiedAppInfo() != ERR_OK || InitNativeProcInfo() != ERR_OK) {
        STANDBYSERVICE_LOGW("calculate proxied app or native process failed");
        ClearProxyRecord();
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    ProxyAppAndProcess(true);
    return ERR_OK;
}

ErrCode RunningLockStrategy::InitProxiedAppInfo()
{
    if (GetAllAppInfos() != ERR_OK || GetAllRunningAppInfo() != ERR_OK) {
        STANDBYSERVICE_LOGW("failed to get all app info");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    if (GetForegroundApplications() !=ERR_OK || GetBackgroundTaskApp() != ERR_OK ||
        GetWorkSchedulerTask() != ERR_OK || GetExemptionConfig() != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    uidBundleNmeMap_.clear();
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetAllAppInfos()
{
    // get all app and set UNRESTRICTED flag to system app.
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
        uidBundleNmeMap_.emplace(info.uid, info.name);
        std::string key = std::to_string(info.uid) + "_" + info.name;
        proxiedAppInfo_.emplace(key, ProxiedProcInfo {info.name, info.uid});
        // system app have exemption
        if (info.isSystemApp) {
            proxiedAppInfo_[key].appExemptionFlag_ |= ExemptionTypeFlag::UNRESTRICTED;
        }
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetAllRunningAppInfo()
{
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos {};
    if (!AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos)) {
        STANDBYSERVICE_LOGE("connect to app manager service failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    // get all running proc of app, add them to proxiedAppInfo_.
    STANDBYSERVICE_LOGI("current running processes size %{public}d", static_cast<int32_t>(allAppProcessInfos.size()));
    std::set<int> runningUids {};
    for (const auto& info : allAppProcessInfos) {
        if (uidBundleNmeMap_.find(info.uid_) == uidBundleNmeMap_.end()) {
            continue;
        }
        runningUids.emplace(info.uid_);
        std::string key = std::to_string(info.uid_) + "_" + uidBundleNmeMap_[info.uid_];
        auto iter = proxiedAppInfo_.find(key);
        if (iter == proxiedAppInfo_.end()) {
            std::tie(iter, std::ignore) = proxiedAppInfo_.emplace(key,
                ProxiedProcInfo {info.processName_, info.uid_});
        }
        iter->second.pids_.emplace(info.pid_);
    }
    // if app is not running, delete its info from proxiedAppInfo_.
    for (auto appInfoIter = proxiedAppInfo_.begin(); appInfoIter != proxiedAppInfo_.end();) {
        if (runningUids.find(appInfoIter->second.uid_) == runningUids.end()) {
            appInfoIter = proxiedAppInfo_.erase(appInfoIter);
        } else {
            ++appInfoIter;
        }
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetWorkSchedulerTask()
{
    #ifdef STANDBY_RSS_WORK_SCHEDULER_ENABLE
    std::list<std::shared_ptr<WorkScheduler::WorkInfo>> workInfos;
    if (WorkScheduler::WorkSchedulerSrvClient::GetInstance().GetAllRunningWorks(workInfos) != ERR_OK) {
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }

    STANDBYSERVICE_LOGD("GetWorkSchedulerTask succeed, size is %{public}d", static_cast<int32_t>(workInfos.size()));
    for (const auto& task : workInfos) {
        std::string key = std::to_string(task->GetUid()) + "_" + task->GetBundleName();
        if (auto iter = proxiedAppInfo_.find(key); iter != proxiedAppInfo_.end()) {
            iter->second.appExemptionFlag_ |= ExemptionTypeFlag::WORK_SCHEDULER;
        }
    }
    #endif
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetForegroundApplications()
{
    std::vector<AppExecFwk::AppStateData> fgApps {};
    if (!AppMgrHelper::GetInstance()->GetForegroundApplications(fgApps)) {
        STANDBYSERVICE_LOGW("get foreground app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    // add foreground flag to app
    for (const auto& appInfo : fgApps) {
        std::string key = std::to_string(appInfo.uid) + "_" + appInfo.bundleName;
        if (auto iter = proxiedAppInfo_.find(key); iter != proxiedAppInfo_.end()) {
            iter->second.appExemptionFlag_ |= ExemptionTypeFlag::FOREGROUND_APP;
        }
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetBackgroundTaskApp()
{
    #ifdef ENABLE_BACKGROUND_TASK_MGR
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> continuousTaskList;
    if (!BackgroundTaskHelper::GetInstance()->GetContinuousTaskApps(continuousTaskList)) {
        STANDBYSERVICE_LOGE("get continuous task app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    STANDBYSERVICE_LOGD("succeed GetContinuousTaskApps, size is %{public}d",
        static_cast<int32_t>(continuousTaskList.size()));
    std::vector<std::shared_ptr<TransientTaskAppInfo>> transientTaskList;
    if (!BackgroundTaskHelper::GetInstance()->GetTransientTaskApps(transientTaskList)) {
        STANDBYSERVICE_LOGE("get transient task app failed");
        return ERR_STRATEGY_DEPENDS_SA_NOT_AVAILABLE;
    }
    // add continuous exemption flag for app with continuous task
    STANDBYSERVICE_LOGD("succeed GetTransientTaskApps, size is %{public}d",
        static_cast<int32_t>(transientTaskList.size()));
    for (const auto& task : continuousTaskList) {
        auto iter = uidBundleNmeMap_.find(task->GetCreatorUid());
        if (iter == uidBundleNmeMap_.end()) {
            continue;
        }
        std::string key = std::to_string(task->GetCreatorUid()) + "_" + iter->second;
        if (auto infoIter = proxiedAppInfo_.find(key); infoIter == proxiedAppInfo_.end()) {
            continue;
        } else {
            infoIter->second.appExemptionFlag_ |= ExemptionTypeFlag::CONTINUOUS_TASK;
        }
    }
    // add transient exemption flag for app with transient task
    for (const auto& task : transientTaskList) {
        std::string key = std::to_string(task->GetUid()) + "_" + task->GetPackageName();
        if (auto iter = proxiedAppInfo_.find(key); iter == proxiedAppInfo_.end()) {
            continue;
        } else {
            iter->second.appExemptionFlag_ |= ExemptionTypeFlag::TRANSIENT_TASK;
        }
    }
    #endif
    return ERR_OK;
}

ErrCode RunningLockStrategy::GetExemptionConfig()
{
    // if app in exemption list, add its exemption flag
    std::vector<AllowInfo> allowInfoList {};
    StandbyServiceImpl::GetInstance()->GetAllowListInner(AllowType::RUNNING_LOCK, allowInfoList,
        ReasonCodeEnum::REASON_APP_API);
    std::set<std::string> allowNameList {};
    for (const auto& info : allowInfoList) {
        allowNameList.emplace(info.GetName());
    }
    for (auto& [key, value] : proxiedAppInfo_) {
        if (allowNameList.find(value.name_) == allowNameList.end()) {
            continue;
        }
        value.appExemptionFlag_ |= ExemptionTypeFlag::EXEMPTION;
    }

    // if app in restrict list, add retricted flag
    std::set<std::string> restrictBundleName {};
    StandbyServiceImpl::GetInstance()->GetEligiableRestrictSet(AllowType::RUNNING_LOCK, "RUNNING_LOCK",
        ReasonCodeEnum::REASON_APP_API, restrictBundleName);
    STANDBYSERVICE_LOGI("running lock restrict app list, size is %{public}d",
        static_cast<int32_t>(restrictBundleName.size()));
    for (auto& [key, value] : proxiedAppInfo_) {
        if (restrictBundleName.find(value.name_) == restrictBundleName.end()) {
            continue;
        }
        value.appExemptionFlag_ |= ExemptionTypeFlag::RESTRICTED;
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::InitNativeProcInfo()
{
    return ERR_OK;
}

ErrCode RunningLockStrategy::ProxyAppAndProcess(bool isProxied)
{
    std::vector<std::pair<int32_t, int32_t>> proxiedAppList;
    for (const auto& [key, value] : proxiedAppInfo_) {
        if (ExemptionTypeFlag::IsExempted(value.appExemptionFlag_)) {
            continue;
        }
        SetProxiedAppList(proxiedAppList, value);
    }
    STANDBYSERVICE_LOGD("proxied app size: %{public}d", static_cast<int32_t>(proxiedAppList.size()));
    ProxyRunningLockList(isProxied, proxiedAppList);
    return ERR_OK;
}

ErrCode RunningLockStrategy::StopProxy(const StandbyMessage& message)
{
    if (!isProxied_) {
        return ERR_STANDBY_CURRENT_STATE_NOT_MATCH;
    }
    uint32_t preState = static_cast<uint32_t>(message.want_->GetIntParam(PREVIOUS_STATE, 0));
    uint32_t curState = static_cast<uint32_t>(message.want_->GetIntParam(CURRENT_STATE, 0));
    if ((curState == StandbyState::MAINTENANCE) && (preState == StandbyState::SLEEP)) {
        // enter maintenance, stop proxy
        ProxyAppAndProcess(false);
        isIdleMaintence_ = true;
    } else if ((curState == StandbyState::SLEEP) && (preState == StandbyState::MAINTENANCE)) {
        isIdleMaintence_ = false;
        // exit maintenance, enter sleep, start proxy
        ProxyAppAndProcess(true);
    } else if (preState == StandbyState::SLEEP || preState == StandbyState::MAINTENANCE) {
        StopProxyInner();
        isProxied_ = false;
        isIdleMaintence_ = false;
    }
    return ERR_OK;
}

ErrCode RunningLockStrategy::StopProxyInner()
{
    ProxyAppAndProcess(false);
    ClearProxyRecord();
    return ERR_OK;
}

ErrCode RunningLockStrategy::UpdateBgTaskAppStatus(const StandbyMessage& message)
{
    std::string type = message.want_->GetStringParam(BG_TASK_TYPE);
    bool started = message.want_->GetBoolParam(BG_TASK_STATUS, false);
    int32_t uid = message.want_->GetIntParam(BG_TASK_UID, 0);
    std::string bundleName = message.want_->GetStringParam(BG_TASK_BUNDLE_NAME);

    STANDBYSERVICE_LOGD("received bgtask status changed, type: %{public}s, isstarted: %{public}d, uid: %{public}d",
        type.c_str(), started, uid);
    if (BGTASK_EXEMPTION_FLAG_MAP.find(type) == BGTASK_EXEMPTION_FLAG_MAP.end()) {
        return ERR_STANDBY_KEY_INFO_NOT_MATCH;
    }

    if (bundleName.empty()) {
        bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    }
    if (started) {
        AddExemptionFlag(uid, bundleName, BGTASK_EXEMPTION_FLAG_MAP.at(type));
    } else {
        RemoveExemptionFlag(uid, bundleName, BGTASK_EXEMPTION_FLAG_MAP.at(type));
    }
    return ERR_OK;
}

// when bgtask or work_scheduler service crash, reset proxy status
void RunningLockStrategy::ResetProxyStatus(const StandbyMessage& message)
{
    if (!isProxied_ || isIdleMaintence_) {
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
    for (const auto& [key, value] : proxiedAppInfo_) {
        if ((value.appExemptionFlag_ & bgTaskFlag) == 0) {
            continue;
        }
        RemoveExemptionFlag(value.uid_, value.name_, (value.appExemptionFlag_ & bgTaskFlag));
    }
    return;
}

void RunningLockStrategy::AddExemptionFlag(int32_t uid, const std::string& name, uint8_t flag)
{
    if (!isProxied_) {
        return;
    }
    std::string mapKey = std::to_string(uid) + "_" + name;
    // if last appExemptionFlag is not exempted, current appExemptionFlag is exempted, unproxy running lock
    if (auto iter = proxiedAppInfo_.find(mapKey); iter != proxiedAppInfo_.end()) {
        auto lastAppExemptionFlag = iter->second.appExemptionFlag_;
        iter->second.appExemptionFlag_ |= flag;
        if (!ExemptionTypeFlag::IsExempted(lastAppExemptionFlag) &&
            ExemptionTypeFlag::IsExempted(iter->second.appExemptionFlag_)) {
            std::vector<std::pair<int32_t, int32_t>> proxiedAppList;
            SetProxiedAppList(proxiedAppList, iter->second);
            ProxyRunningLockList(false, proxiedAppList);
        }
    }
}

void RunningLockStrategy::RemoveExemptionFlag(int32_t uid, const std::string& name, uint8_t flag)
{
    std::string mapKey = std::to_string(uid) + "_" + name;
    auto iter = proxiedAppInfo_.find(mapKey);
    if (iter == proxiedAppInfo_.end()) {
        return;
    }

    // if last appExemptionFlag is exempted, current appExemptionFlag is unexempted, proxy running lock
    auto lastAppExemptionFlag = iter->second.appExemptionFlag_;
    iter->second.appExemptionFlag_ &= (~flag);
    if (ExemptionTypeFlag::IsExempted(iter->second.appExemptionFlag_) ||
        !ExemptionTypeFlag::IsExempted(lastAppExemptionFlag)) {
        return;
    }

    std::vector<std::pair<int32_t, int32_t>> proxiedAppList;
    SetProxiedAppList(proxiedAppList, iter->second);
    ProxyRunningLockList(true, proxiedAppList);
}

void RunningLockStrategy::ClearProxyRecord()
{
    proxiedAppInfo_.clear();
    uidBundleNmeMap_.clear();
}

// when app is created, add app info to cache
void RunningLockStrategy::GetAndCreateAppInfo(uint32_t uid, uint32_t pid, const std::string& bundleName)
{
    const std::string mapKey = std::to_string(uid) + "_" + bundleName;
    auto iter = proxiedAppInfo_.find(mapKey);
    if (iter != proxiedAppInfo_.end()) {
        iter->second.pids_.emplace(pid);
        return;
    }

    std::tie(iter, std::ignore) = proxiedAppInfo_.emplace(mapKey, ProxiedProcInfo {bundleName, uid});
    iter->second.pids_.emplace(pid);

    bool isSystemApp {false};
    if (BundleManagerHelper::GetInstance()->CheckIsSystemAppByUid(uid, isSystemApp) && isSystemApp) {
        iter->second.appExemptionFlag_ |= ExemptionTypeFlag::UNRESTRICTED;
    }
    GetExemptionConfigForApp(iter->second, bundleName);
}

ErrCode RunningLockStrategy::GetExemptionConfigForApp(ProxiedProcInfo& appInfo, const std::string& bundleName)
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

void RunningLockStrategy::SetProxiedAppList(std::vector<std::pair<int32_t, int32_t>>& proxiedAppList,
    const ProxiedProcInfo& info)
{
    for (auto& pid : info.pids_) {
        proxiedAppList.emplace_back(std::make_pair(pid, info.uid_));
    }
}

void RunningLockStrategy::ProxyRunningLockList(bool isProxied,
    const std::vector<std::pair<int32_t, int32_t>>& proxiedAppList)
{
    if (proxiedAppList.empty()) {
        return;
    }
    // in maintenance state, disallow proxy running lock
    if (isIdleMaintence_) {
        STANDBYSERVICE_LOGI("current is idle maintenance, can not proxy running lock");
        return;
    }
    #ifdef STANDBY_POWER_MANAGER_ENABLE
    if (!PowerMgr::PowerMgrClient::GetInstance().ProxyRunningLocks(isProxied, proxiedAppList)) {
        STANDBYSERVICE_LOGW("failed to ProxyRunningLockList");
    }
    #endif
}

void RunningLockStrategy::HandleProcessStatusChanged(const StandbyMessage& message)
{
    if (!isProxied_) {
        STANDBYSERVICE_LOGD("RunningLockStrategy is not in proxy, do not need process");
        return;
    }
    int32_t uid = message.want_->GetIntParam("uid", -1);
    int32_t pid = message.want_->GetIntParam("pid", -1);
    std::string bundleName = message.want_->GetStringParam("name");
    bool isCreated = message.want_->GetBoolParam("isCreated", false);

    auto key = std::to_string(uid) + "_" + bundleName;
    if (isCreated) {
        // if process is created
        GetAndCreateAppInfo(uid, pid, bundleName);
        if (!ExemptionTypeFlag::IsExempted(proxiedAppInfo_[key].appExemptionFlag_)) {
            ProxyRunningLockList(true, {std::make_pair(pid, uid)});
        }
    } else {
        auto iter = proxiedAppInfo_.find(key);
        if (iter == proxiedAppInfo_.end()) {
            return;
        }
        if (!ExemptionTypeFlag::IsExempted(proxiedAppInfo_[key].appExemptionFlag_)) {
            ProxyRunningLockList(false, {std::make_pair(pid, uid)});
        }
        iter->second.pids_.erase(pid);
        if (iter->second.pids_.empty()) {
            proxiedAppInfo_.erase(iter);
        }
    }
}

void RunningLockStrategy::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr[DUMP_FIRST_PARAM] == DUMP_DETAIL_INFO &&
        argsInStr[DUMP_SECOND_PARAM] == DUMP_STRATGY_DETAIL) {
        DumpShowDetailInfo(argsInStr, result);
    }
}

// dump detail info of running lock strategy
void RunningLockStrategy::DumpShowDetailInfo(const std::vector<std::string>& argsInStr, std::string& result)
{
    result.append("=================RunningLock======================\n");
    result.append("Running Lock Strategy:\n").append("isProxied: " + std::to_string(isProxied_))
        .append(" isIdleMaintence: " + std::to_string(isIdleMaintence_)).append("\n");
    result.append("proxied app info: \n");
    for (const auto& [key, value] : proxiedAppInfo_) {
        result.append("key: ").append(key).append(" name: ").append(value.name_).append(" uid: ")
            .append(std::to_string(value.uid_)).append(" pid_size: ")
            .append(std::to_string(value.pids_.size())).append(" exemption flag: ")
            .append(std::to_string(value.appExemptionFlag_)).append("\n");
        if (value.pids_.empty()) {
            continue;
        }
        result.append("pids list: ");
        for (const auto pid : value.pids_) {
            result.append(" ").append(std::to_string(pid));
        }
        result.append("\n");
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS