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

#include "standby_service_impl.h"

#include <algorithm>
#include <dlfcn.h>
#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ability_manager_helper.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "allow_type.h"
#include "app_mgr_helper.h"
#include "bundle_manager_helper.h"
#include "common_event_observer.h"
#include "common_event_support.h"
#include "event_runner.h"
#include "istandby_service.h"
#include "json_utils.h"
#include "standby_config_manager.h"
#include "standby_service.h"
#include "standby_service_log.h"
#include "system_ability_definition.h"
#include "timed_task.h"
#include "time_provider.h"
#include "time_service_client.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace DevStandbyMgr {
namespace {
const std::string ALLOW_RECORD_FILE_PATH = "/data/service/el1/public/device_standby/allow_record";
const std::string STANDBY_MSG_HANDLER = "StandbyMsgHandler";
const std::string ON_PLUGIN_REGISTER = "OnPluginRegister";
#ifdef __LP64__
const std::string SYSTEM_SO_PATH = "/system/lib64/";
#else
const std::string SYSTEM_SO_PATH = "/system/lib/";
#endif
const std::string STANDBY_EXEMPTION_PERMISSION = "ohos.permission.DEVICE_STANDBY_EXEMPTION";
const uint32_t EXEMPT_ALL_RESOURCES = 100;
const std::string COMMON_EVENT_TIMER_SA_ABILITY = "COMMON_EVENT_TIMER_SA_ABILITY";
const uint32_t ONE_SECOND = 1000;
}

IMPLEMENT_SINGLE_INSTANCE(StandbyServiceImpl);

StandbyServiceImpl::StandbyServiceImpl() {}

StandbyServiceImpl::~StandbyServiceImpl() {}

bool StandbyServiceImpl::Init()
{
    auto runner = AppExecFwk::EventRunner::Create(STANDBY_MSG_HANDLER);
    if (runner == nullptr) {
        STANDBYSERVICE_LOGE("dev standby service runner create failed");
        return false;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (!handler_) {
        STANDBYSERVICE_LOGE("standby msg handler create failed");
        return false;
    }
    if (StandbyConfigManager::GetInstance()->Init() != ERR_OK) {
        STANDBYSERVICE_LOGE("failed to init device standby config manager");
        return false;
    }
    if (RegisterPlugin(StandbyConfigManager::GetInstance()->GetPluginName()) != ERR_OK
        && RegisterPlugin(DEFAULT_PLUGIN_NAME) != ERR_OK) {
        STANDBYSERVICE_LOGE("register plugin failed");
        return false;
    }
    STANDBYSERVICE_LOGI("register plugin secceed, dev standby service implement finish Init");
    return true;
}

void StandbyServiceImpl::InitReadyState()
{
    STANDBYSERVICE_LOGD("start init necessary plugin");
    handler_->PostTask([this]() {
        if (isServiceReady_.load()) {
            STANDBYSERVICE_LOGW("standby service is already ready, do not need repeat");
            return;
        }
        if (!standbyStateManager_->Init()) {
            STANDBYSERVICE_LOGE("standby state manager init failed");
            return;
        }
        if (!strategyManager_->Init()) {
            STANDBYSERVICE_LOGE("strategy plugin init failed");
            return;
        }
        if (!constraintManager_->Init()) {
            STANDBYSERVICE_LOGE("constraint plugin init failed");
            return;
        }
        if (!listenerManager_->Init() || listenerManager_->StartListener() != ERR_OK) {
            STANDBYSERVICE_LOGE("listener plugin init failed");
            return;
        }

        RegisterTimeObserver();
        ParsePersistentData();
        isServiceReady_.store(true);

        StandbyService::GetInstance()->AddPluginSysAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
        StandbyService::GetInstance()->AddPluginSysAbilityListener(WORK_SCHEDULE_SERVICE_ID);
        }, AppExecFwk::EventQueue::Priority::HIGH);
}

ErrCode StandbyServiceImpl::RegisterCommEventObserver()
{
    STANDBYSERVICE_LOGI("register common event observer");
    std::lock_guard<std::mutex> lock(eventObserverMutex_);
    if (commonEventObserver_) {
        return ERR_STANDBY_OBSERVER_ALREADY_EXIST;
    }
    commonEventObserver_ = CommonEventObserver::CreateCommonEventObserver(handler_);
    if (!commonEventObserver_) {
        STANDBYSERVICE_LOGE("register common event observer failed");
        return ERR_STANDBY_OBSERVER_INIT_FAILED;
    }
    if (!commonEventObserver_->Subscribe()) {
        STANDBYSERVICE_LOGE("SubscribeCommonEvent failed");
        return ERR_STANDBY_OBSERVER_INIT_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceImpl::RegisterAppStateObserver()
{
    std::lock_guard<std::mutex> lock(appStateObserverMutex_);
    if (appStateObserver_) {
        return ERR_STANDBY_OBSERVER_ALREADY_EXIST;
    }
    appStateObserver_ = new (std::nothrow) AppStateObserver(handler_);
    if (!appStateObserver_) {
        STANDBYSERVICE_LOGE("malloc appStateObserver failed");
        return ERR_STANDBY_OBSERVER_INIT_FAILED;
    }
    if (!AppMgrHelper::GetInstance()->SubscribeObserver(appStateObserver_)) {
        STANDBYSERVICE_LOGE("subscribe appStateObserver failed");
        return ERR_STANDBY_OBSERVER_INIT_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceImpl::UnregisterAppStateObserver()
{
    STANDBYSERVICE_LOGI("unregister app state observer");
    std::lock_guard<std::mutex> lock(appStateObserverMutex_);
    if (appStateObserver_) {
        AppMgrHelper::GetInstance()->UnsubscribeObserver(appStateObserver_);
        appStateObserver_ = nullptr;
    }
    return ERR_OK;
}

void StandbyServiceImpl::DayNightSwitchCallback()
{
    handler_->PostTask([standbyImpl = shared_from_this()]() {
        STANDBYSERVICE_LOGD("start day and night switch");
        if (!standbyImpl->isServiceReady_.load()) {
            STANDBYSERVICE_LOGW("standby service is not ready");
            if (!TimedTask::StartDayNightSwitchTimer(standbyImpl->dayNightSwitchTimerId_)) {
                standbyImpl->ResetTimeObserver();
            }
            return;
        }
        auto curState = standbyImpl->standbyStateManager_->GetCurState();
        if (curState == StandbyState::SLEEP) {
            StandbyMessage standbyMessage {StandbyMessageType::RES_CTRL_CONDITION_CHANGED};
            standbyMessage.want_ = AAFwk::Want {};
            uint32_t condition = TimeProvider::GetCondition();
            standbyMessage.want_->SetParam(RES_CTRL_CONDITION, static_cast<int32_t>(condition));
            standbyImpl->DispatchEvent(standbyMessage);
        }
        if (!TimedTask::StartDayNightSwitchTimer(standbyImpl->dayNightSwitchTimerId_)) {
            STANDBYSERVICE_LOGE("start day and night switch timer failed");
            standbyImpl->ResetTimeObserver();
        }
    });
}

ErrCode StandbyServiceImpl::RegisterTimeObserver()
{
    std::lock_guard<std::recursive_mutex> lock(timerObserverMutex_);
    handler_->PostTask([=]() {
            StandbyMessage message(StandbyMessageType::COMMON_EVENT, COMMON_EVENT_TIMER_SA_ABILITY);
            StandbyServiceImpl::GetInstance()->DispatchEvent(message);
        }, ONE_SECOND);
    if (dayNightSwitchTimerId_ > 0) {
        return ERR_STANDBY_OBSERVER_ALREADY_EXIST;
    }
    auto callBack = [standbyImpl = shared_from_this()]() {
        standbyImpl->DayNightSwitchCallback();
    };
    if (!TimedTask::RegisterDayNightSwitchTimer(dayNightSwitchTimerId_, false, 0, callBack)) {
        STANDBYSERVICE_LOGE("RegisterTimer failed");
        return ERR_STANDBY_OBSERVER_INIT_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceImpl::UnregisterCommEventObserver()
{
    STANDBYSERVICE_LOGI("unregister common event observer");
    std::lock_guard<std::mutex> lock(eventObserverMutex_);
    if (commonEventObserver_) {
        commonEventObserver_->Unsubscribe();
        commonEventObserver_.reset();
    }
    return ERR_OK;
}

ErrCode StandbyServiceImpl::UnregisterTimeObserver()
{
    std::lock_guard<std::recursive_mutex> lock(timerObserverMutex_);
    if (!MiscServices::TimeServiceClient::GetInstance()->StopTimer(dayNightSwitchTimerId_)) {
        STANDBYSERVICE_LOGE("day and night switch observer stop failed");
    }
    if (!MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(dayNightSwitchTimerId_)) {
        STANDBYSERVICE_LOGE("day and night switch observer destroy failed");
    }
    dayNightSwitchTimerId_ = 0;
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ResetTimeObserver()
{
    std::lock_guard<std::recursive_mutex> lock(timerObserverMutex_);
    if (UnregisterTimeObserver() != ERR_OK || RegisterTimeObserver() != ERR_OK) {
        STANDBYSERVICE_LOGE("day and night switch observer reset failed");
        return ERR_STANDBY_OBSERVER_RESET_FAILED;
    }
    return ERR_OK;
}


ErrCode StandbyServiceImpl::RegisterPlugin(const std::string& pluginName)
{
    STANDBYSERVICE_LOGI("start register plugin %{public}s", pluginName.c_str());
    std::string realPluginName {""};
    if (!JsonUtils::GetRealPath(SYSTEM_SO_PATH + pluginName, realPluginName)) {
        STANDBYSERVICE_LOGW("failed to get valid plugin path");
        return ERR_STANDBY_PLUGIN_NOT_EXIST;
    }
    if (strncmp(realPluginName.c_str(), SYSTEM_SO_PATH.c_str(), SYSTEM_SO_PATH.size()) != 0) {
        STANDBYSERVICE_LOGW("plugin must in system directory");
        return ERR_STANDBY_PLUGIN_NOT_EXIST;
    }
    registerPlugin_ = dlopen(realPluginName.c_str(), RTLD_NOW);
    if (!registerPlugin_) {
        dlclose(registerPlugin_);
        STANDBYSERVICE_LOGE("failed to open plugin %{public}s", realPluginName.c_str());
        return ERR_STANDBY_PLUGIN_NOT_EXIST;
    }
    void* pluginFunc = dlsym(registerPlugin_, ON_PLUGIN_REGISTER.c_str());
    if (!pluginFunc) {
        dlclose(registerPlugin_);
        STANDBYSERVICE_LOGE("failed to find extern func of plugin %{public}s", realPluginName.c_str());
        return ERR_STANDBY_PLUGIN_NOT_EXIST;
    }
    auto onPluginInitFunc = reinterpret_cast<bool (*)()>(pluginFunc);
    if (!onPluginInitFunc()) {
        dlclose(registerPlugin_);
        return ERR_STANDBY_PLUGIN_NOT_AVAILABLE;
    }
    return ERR_OK;
}

void StandbyServiceImpl::RegisterPluginInner(IConstraintManagerAdapter* constraintManager,
    IListenerManagerAdapter* listenerManager,
    IStrategyManagerAdapter* strategyManager,
    IStateManagerAdapter* stateManager)
{
    constraintManager_ = std::shared_ptr<IConstraintManagerAdapter>(constraintManager);
    listenerManager_ = std::shared_ptr<IListenerManagerAdapter>(listenerManager);
    strategyManager_ = std::shared_ptr<IStrategyManagerAdapter>(strategyManager);
    standbyStateManager_ = std::shared_ptr<IStateManagerAdapter>(stateManager);
}

std::shared_ptr<AppExecFwk::EventHandler>& StandbyServiceImpl::GetHandler()
{
    return handler_;
}

std::shared_ptr<IConstraintManagerAdapter>& StandbyServiceImpl::GetConstraintManager()
{
    return constraintManager_;
}

std::shared_ptr<IListenerManagerAdapter>& StandbyServiceImpl::GetListenerManager()
{
    return listenerManager_;
}

std::shared_ptr<IStrategyManagerAdapter>& StandbyServiceImpl::GetStrategyManager()
{
    return strategyManager_;
}

std::shared_ptr<IStateManagerAdapter>& StandbyServiceImpl::GetStateManager()
{
    return standbyStateManager_;
}

void StandbyServiceImpl::UninitReadyState()
{
    handler_->PostSyncTask([this]() {
        if (!isServiceReady_.load()) {
            STANDBYSERVICE_LOGW("standby service is already not ready, do not need uninit");
            return;
        }
        STANDBYSERVICE_LOGE("start uninit necessary observer");
        listenerManager_->UnInit();
        constraintManager_->UnInit();
        strategyManager_->UnInit();
        standbyStateManager_->UnInit();
        isServiceReady_.store(false);
        }, AppExecFwk::EventQueue::Priority::HIGH);
}

bool StandbyServiceImpl::ParsePersistentData()
{
    STANDBYSERVICE_LOGD("service start, parse persistent data");
    std::unordered_map<int32_t, std::string> pidNameMap {};
    GetPidAndProcName(pidNameMap);
    if (pidNameMap.empty()) {
        return false;
    }
    nlohmann::json root;
    if (!JsonUtils::LoadJsonValueFromFile(root, ALLOW_RECORD_FILE_PATH)) {
        STANDBYSERVICE_LOGE("failed to load allow record from file");
        return false;
    }

    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    for (auto iter = root.begin(); iter != root.end(); ++iter) {
        std::shared_ptr<AllowRecord> recordPtr = std::make_shared<AllowRecord>();
        if (recordPtr->ParseFromJson(iter.value())) {
            allowInfoMap_.emplace(iter.key(), recordPtr);
        }
    }
    for (auto iter = allowInfoMap_.begin(); iter != allowInfoMap_.end();) {
        auto pidNameIter = pidNameMap.find(iter->second->pid_);
        if (pidNameIter == pidNameMap.end() || pidNameIter->second != iter->second->name_) {
            allowInfoMap_.erase(iter++);
        } else {
            iter++;
        }
    }

    STANDBYSERVICE_LOGI("after reboot, allowInfoMap_ size is %{public}d", static_cast<int32_t>(allowInfoMap_.size()));
    RecoverTimeLimitedTask();
    DumpPersistantData();
    return true;
}

void StandbyServiceImpl::GetPidAndProcName(std::unordered_map<int32_t, std::string>& pidNameMap)
{
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos {};
    if (!AppMgrHelper::GetInstance()->GetAllRunningProcesses(allAppProcessInfos)) {
        STANDBYSERVICE_LOGE("connect to app manager service failed");
        return;
    }
    STANDBYSERVICE_LOGD("GetAllRunningProcesses result size is %{public}d",
        static_cast<int32_t>(allAppProcessInfos.size()));
    for (const auto& info : allAppProcessInfos) {
        pidNameMap.emplace(info.pid_, info.processName_);
    }
    std::list<SystemProcessInfo> systemProcessInfos {};
    if (!AbilityManagerHelper::GetInstance()->GetRunningSystemProcess(systemProcessInfos)) {
        STANDBYSERVICE_LOGE("connect to app ability service failed");
        return;
    }
    STANDBYSERVICE_LOGD("GetRunningSystemProcess result size is %{public}d",
        static_cast<int32_t>(systemProcessInfos.size()));
    for (const auto& info : systemProcessInfos) {
        pidNameMap.emplace(info.pid, info.processName);
    }
}

void StandbyServiceImpl::RecoverTimeLimitedTask()
{
    STANDBYSERVICE_LOGD("start to recovery delayed task");
    const auto &mgr = shared_from_this();
    for (auto iter = allowInfoMap_.begin(); iter != allowInfoMap_.end(); ++iter) {
        auto &allowTimeList = iter->second->allowTimeList_;
        for (auto allowTimeIter = allowTimeList.begin(); allowTimeIter != allowTimeList.end(); ++allowTimeIter) {
            auto task = [mgr, uid = iter->second->uid_, name = iter->second->name_] () {
                mgr->UnapplyAllowResInner(uid, name, MAX_ALLOW_TYPE_NUMBER, false);
            };
            int32_t timeOut = static_cast<int32_t>(allowTimeIter->endTime_ -
                MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs());
            handler_->PostTask(task, std::max(0, timeOut));
        }
    }
}

void StandbyServiceImpl::DumpPersistantData()
{
    nlohmann::json root;
    STANDBYSERVICE_LOGD("dump persistant data");
    for (auto& [uid, allowInfo] : allowInfoMap_) {
        root[uid] = allowInfo->ParseToJson();
    }
    JsonUtils::DumpJsonValueToFile(root, ALLOW_RECORD_FILE_PATH);
}

void StandbyServiceImpl::UnInit()
{
    if (!registerPlugin_) {
        dlclose(registerPlugin_);
        registerPlugin_ = nullptr;
    }
    STANDBYSERVICE_LOGI("succeed to clear stawndby service implement");
}

bool StandbyServiceImpl::CheckAllowTypeInfo(uint32_t allowType)
{
    return allowType > 0 && allowType <= MAX_ALLOW_TYPE_NUMBER;
}

ErrCode StandbyServiceImpl::RemoveAppAllowRecord(int32_t uid, const std::string &bundleName, bool resetAll)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("app died, uid: %{public}d, bundleName: %{public}s", uid, bundleName.c_str());
    int allowType = resetAll ? MAX_ALLOW_TYPE_NUMBER : (MAX_ALLOW_TYPE_NUMBER ^ AllowType::TIMER ^
        AllowType::WORK_SCHEDULER);
    this->UnapplyAllowResInner(uid, bundleName, allowType, true);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::CheckCallerPermission(uint32_t reasonCode)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    STANDBYSERVICE_LOGD("check caller permission, uid of caller is %{public}d", uid);
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    if (Security::AccessToken::AccessTokenKit::GetTokenType(tokenId)
        == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return IsSystemAppWithPermission(uid, tokenId, reasonCode);
    }
    return CheckNativePermission(tokenId);
}

ErrCode StandbyServiceImpl::IsSystemAppWithPermission(int32_t uid,
    Security::AccessToken::AccessTokenID tokenId, uint32_t reasonCode)
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, STANDBY_EXEMPTION_PERMISSION)
        != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        STANDBYSERVICE_LOGE("CheckPermission: ohos.permission.DEVICE_STANDBY_EXEMPTION failed");
        return ERR_STANDBY_PERMISSION_DENIED;
    }

    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    bool isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    if (!isSystemApp) {
        STANDBYSERVICE_LOGE("uid %{public}d is not system app", uid);
        return ERR_STANDBY_NOT_SYSTEM_APP;
    }
    if (reasonCode != ReasonCodeEnum::REASON_APP_API) {
        STANDBYSERVICE_LOGE("reasonCode error, uid %{public}d  must be app api", uid);
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceImpl::CheckNativePermission(Security::AccessToken::AccessTokenID tokenId)
{
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::TypeATokenTypeEnum::TOKEN_NATIVE) {
        return ERR_OK;
    }
    if (tokenFlag == Security::AccessToken::TypeATokenTypeEnum::TOKEN_SHELL) {
        return ERR_OK;
    }
    return ERR_STANDBY_PERMISSION_DENIED;
}

uint32_t StandbyServiceImpl::GetExemptedResourceType(uint32_t resourceType)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    const std::vector<int32_t>& resourcesApply = QueryRunningResourcesApply(uid, bundleName);

    uint32_t exemptedResourceType = 0;
    if (resourcesApply.empty()) {
        return exemptedResourceType;
    }

    if (std::find(resourcesApply.begin(), resourcesApply.end(), EXEMPT_ALL_RESOURCES) != resourcesApply.end()) {
        return resourceType;
    }

    // traverse resourcesApply and get exempted resource type
    for (const uint32_t resourceType : resourcesApply) {
        if (resourceType <= EXEMPT_ALL_RESOURCES || resourceType > EXEMPT_ALL_RESOURCES + MAX_ALLOW_TYPE_NUM + 1) {
            continue;
        }
        // maps number in resourceApply to resourceType defined in allow_type.h
        exemptedResourceType |= (1 << (resourceType - EXEMPT_ALL_RESOURCES - 1));
    }
    exemptedResourceType &= resourceType;

    return exemptedResourceType;
}

// meaning of number in resourcesApply list: 100 - all type of resources, 101 - NETWORK,
// 102 - RUNNING_LOCK, 103 - TIMER, 104 - WORK_SCHEDULER, 105 - AUTO_SYNC, 106 - PUSH, 107 - FREEZE
std::vector<int32_t> StandbyServiceImpl::QueryRunningResourcesApply(const int32_t uid, const std::string &bundleName)
{
    AppExecFwk::ApplicationInfo applicationInfo;
    if (!BundleManagerHelper::GetInstance()->GetApplicationInfo(bundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, GetUserIdByUid(uid), applicationInfo)) {
        STANDBYSERVICE_LOGE("failed to get applicationInfo, bundleName is %{public}s", bundleName.c_str());
        return {};
    }
    STANDBYSERVICE_LOGD("size of applicationInfo.resourcesApply is %{public}d",
        static_cast<int32_t>(applicationInfo.resourcesApply.size()));
    return applicationInfo.resourcesApply;
}

int32_t StandbyServiceImpl::GetUserIdByUid(int32_t uid)
{
    const int32_t BASE_USER_RANGE = 200000;
    return uid / BASE_USER_RANGE;
}

ErrCode StandbyServiceImpl::SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    STANDBYSERVICE_LOGI("add %{public}s subscriber to stanby service", subscriber->GetSubscriberName().c_str());
    if (CheckNativePermission(OHOS::IPCSkeleton::GetCallingTokenID()) != ERR_OK) {
        STANDBYSERVICE_LOGW("invoker is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    const auto& strategyConfigList = StandbyConfigManager::GetInstance()->GetStrategyConfigList();
    auto item = std::find(strategyConfigList.begin(), strategyConfigList.end(), subscriber->GetSubscriberName());
    if (item == strategyConfigList.end()) {
        STANDBYSERVICE_LOGI("%{public}s is not exist in StrategyConfigList", subscriber->GetSubscriberName().c_str());
        return ERR_STANDBY_STRATEGY_NOT_DEPLOY;
    }
    return StandbyStateSubscriber::GetInstance()->AddSubscriber(subscriber);
}

ErrCode StandbyServiceImpl::UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    STANDBYSERVICE_LOGI("add subscriber to stanby service succeed");
    if (CheckNativePermission(OHOS::IPCSkeleton::GetCallingTokenID()) != ERR_OK) {
        STANDBYSERVICE_LOGW("invoker is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    return StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber);
}

ErrCode StandbyServiceImpl::ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start AddAllowList");
    if (auto checkRet = CheckCallerPermission(resourceRequest->GetReasonCode()); checkRet != ERR_OK) {
        return checkRet;
    }

    // update allow type according to configuration
    if (Security::AccessToken::AccessTokenKit::GetTokenType(OHOS::IPCSkeleton::GetCallingTokenID())
        == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        resourceRequest->SetAllowType(GetExemptedResourceType(resourceRequest->GetAllowType()));
    }

    if (!CheckAllowTypeInfo(resourceRequest->GetAllowType()) || resourceRequest->GetUid() < 0) {
        STANDBYSERVICE_LOGE("resourceRequest param is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    if (resourceRequest->GetDuration() < 0) {
        STANDBYSERVICE_LOGE("duration param is invalid");
        return ERR_DURATION_INVALID;
    }
    int32_t pid = IPCSkeleton::GetCallingPid();
    ApplyAllowResInner(resourceRequest, pid);
    return ERR_OK;
}

void StandbyServiceImpl::ApplyAllowResInner(const sptr<ResourceRequest>& resourceRequest, int32_t pid)
{
    STANDBYSERVICE_LOGI("apply res inner, uid: %{public}d, name: %{public}s, allowType: %{public}u,"\
        " duration: %{public}d, reason: %{public}s", resourceRequest->GetUid(),
        resourceRequest->GetName().c_str(), resourceRequest->GetAllowType(),
        resourceRequest->GetDuration(), resourceRequest->GetReason().c_str());

    int32_t uid = resourceRequest->GetUid();
    const std::string& name = resourceRequest->GetName();
    std::string keyStr = std::to_string(uid) + "_" + name;
    uint32_t preAllowType = 0;

    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    auto iter = allowInfoMap_.find(keyStr);
    if (iter == allowInfoMap_.end()) {
        std::tie(iter, std::ignore) =
            allowInfoMap_.emplace(keyStr, std::make_shared<AllowRecord>(uid, pid, name, 0));
        iter->second->reasonCode_ = resourceRequest->GetReasonCode();
    } else {
        preAllowType = iter->second->allowType_;
        iter->second->pid_ = pid;
    }
    UpdateRecord(iter->second, resourceRequest);
    if (preAllowType != iter->second->allowType_) {
        uint32_t alowTypeDiff = iter->second->allowType_ ^ (preAllowType &
            iter->second->allowType_);
        STANDBYSERVICE_LOGD("after update record, there is added exemption type: %{public}d",
            alowTypeDiff);
        StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(uid, name, alowTypeDiff, true);
        NotifyAllowListChanged(uid, name, alowTypeDiff, true);
    }
    if (iter->second->allowType_ == 0) {
        STANDBYSERVICE_LOGD("%{public}s does not have valid record, delete record", keyStr.c_str());
        allowInfoMap_.erase(iter);
    }
    DumpPersistantData();
}

void StandbyServiceImpl::UpdateRecord(std::shared_ptr<AllowRecord>& allowRecord,
    const sptr<ResourceRequest>& resourceRequest)
{
    STANDBYSERVICE_LOGD("start UpdateRecord");
    int32_t uid = resourceRequest->GetUid();
    const std::string& name = resourceRequest->GetName();
    uint32_t allowType = resourceRequest->GetAllowType();
    bool isApp = (resourceRequest->GetReasonCode() == ReasonCodeEnum::REASON_APP_API);
    int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    int64_t endTime {0};
    uint32_t condition = TimeProvider::GetCondition();
    for (uint32_t allowTypeIndex = 0; allowTypeIndex < MAX_ALLOW_TYPE_NUM; ++allowTypeIndex) {
        uint32_t allowNumber = allowType & (1 << allowTypeIndex);
        if (allowNumber == 0) {
            continue;
        }
        int64_t maxDuration = 0;
        if (allowNumber != AllowType::WORK_SCHEDULER) {
            maxDuration = std::min(resourceRequest->GetDuration(), StandbyConfigManager::GetInstance()->
                GetMaxDuration(name, AllowTypeName[allowTypeIndex], condition, isApp)) * TimeConstant::MSEC_PER_SEC;
        } else {
            maxDuration = resourceRequest->GetDuration() * TimeConstant::MSEC_PER_SEC;
        }
        if (maxDuration <= 0) {
            continue;
        }
        endTime = curTime + maxDuration;
        auto& allowTimeList = allowRecord->allowTimeList_;
        auto findRecordTask = [allowTypeIndex](const auto& it) { return it.allowTypeIndex_ == allowTypeIndex; };
        auto it = std::find_if(allowTimeList.begin(), allowTimeList.end(), findRecordTask);
        if (it == allowTimeList.end()) {
            allowTimeList.emplace_back(AllowTime {allowTypeIndex, endTime, resourceRequest->GetReason()});
        } else {
            it->reason_ = resourceRequest->GetReason();
            it->endTime_ = std::max(it->endTime_, endTime);
        }
        allowRecord->allowType_ = (allowRecord->allowType_ | allowNumber);
        auto task = [this, uid, name, allowType] () {
            this->UnapplyAllowResInner(uid, name, allowType, false);
        };
        handler_->PostTask(task, maxDuration);
    }
    STANDBYSERVICE_LOGE("update end time of allow list");
}

ErrCode StandbyServiceImpl::UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start UnapplyAllowResource");
    if (auto checkRet = CheckCallerPermission(resourceRequest->GetReasonCode()); checkRet != ERR_OK) {
        return checkRet;
    }

    // update allow type according to configuration
    if (Security::AccessToken::AccessTokenKit::GetTokenType(OHOS::IPCSkeleton::GetCallingTokenID())
        == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        resourceRequest->SetAllowType(GetExemptedResourceType(resourceRequest->GetAllowType()));
    }

    if (!CheckAllowTypeInfo(resourceRequest->GetAllowType()) || resourceRequest->GetUid() < 0) {
        STANDBYSERVICE_LOGE("param of resourceRequest is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    UnapplyAllowResInner(resourceRequest->GetUid(), resourceRequest->GetName(), resourceRequest->GetAllowType(), true);
    return ERR_OK;
}

void StandbyServiceImpl::UnapplyAllowResInner(int32_t uid, const std::string& name,
    uint32_t allowType, bool removeAll)
{
    STANDBYSERVICE_LOGD("start UnapplyAllowResInner, uid is %{public}d, allowType is %{public}d, removeAll is "\
        "%{public}d", uid, allowType, removeAll);
    std::string keyStr = std::to_string(uid) + "_" + name;

    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    auto iter = allowInfoMap_.find(keyStr);
    if (iter == allowInfoMap_.end()) {
        STANDBYSERVICE_LOGD("uid has no corresponding allow list");
        return;
    }
    if ((allowType & iter->second->allowType_) == 0) {
        STANDBYSERVICE_LOGD("allow list has no corresponding allow type");
        return;
    }
    auto& allowRecordPtr = iter->second;
    auto& allowTimeList = allowRecordPtr->allowTimeList_;
    uint32_t removedNumber = 0;
    int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    for (auto it = allowTimeList.begin(); it != allowTimeList.end();) {
        uint32_t allowNumber = allowType & (1 << it->allowTypeIndex_);
        if (allowNumber != 0 && (removeAll || curTime >= it->endTime_)) {
            it = allowTimeList.erase(it);
            removedNumber |= allowNumber;
        } else {
            ++it;
        }
    }
    STANDBYSERVICE_LOGD("remove allow list, uid: %{public}d, type: %{public}u", uid, removedNumber);
    if (removedNumber == 0) {
        STANDBYSERVICE_LOGW("none member of the allow list should be removed");
        return;
    }
    if (removedNumber == allowRecordPtr->allowType_) {
        allowInfoMap_.erase(keyStr);
        STANDBYSERVICE_LOGI("allow list has been delete");
    }
    allowRecordPtr->allowType_ = allowRecordPtr->allowType_ - removedNumber;
    StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(uid, name, removedNumber, false);
    NotifyAllowListChanged(uid, name, removedNumber, false);
    DumpPersistantData();
}

void StandbyServiceImpl::OnProcessStatusChanged(int32_t uid, int32_t pid, const std::string& bundleName, bool isCreated)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return;
    }
    STANDBYSERVICE_LOGD("process status change, uid: %{public}d, pid: %{public}d, name: %{public}s, alive: %{public}d",
        uid, pid, bundleName.c_str(), isCreated);
    StandbyMessage standbyMessage {StandbyMessageType::PROCESS_STATE_CHANGED};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("uid", uid);
    standbyMessage.want_->SetParam("pid", pid);
    standbyMessage.want_->SetParam("name", bundleName);
    standbyMessage.want_->SetParam("isCreated", isCreated);
    DispatchEvent(standbyMessage);
}

void StandbyServiceImpl::NotifyAllowListChanged(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    StandbyMessage standbyMessage {StandbyMessageType::ALLOW_LIST_CHANGED};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("uid", uid);
    standbyMessage.want_->SetParam("name", name);
    standbyMessage.want_->SetParam("allowType", static_cast<int>(allowType));
    standbyMessage.want_->SetParam("added", added);
    DispatchEvent(standbyMessage);
}

ErrCode StandbyServiceImpl::GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
    uint32_t reasonCode)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }

    STANDBYSERVICE_LOGD("start GetAllowList");
    if (auto checkRet = CheckCallerPermission(reasonCode); checkRet != ERR_OK) {
        return checkRet;
    }

    if (!CheckAllowTypeInfo(allowType)) {
        STANDBYSERVICE_LOGE("allowtype param is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    GetAllowListInner(allowType, allowInfoList, reasonCode);
    return ERR_OK;
}

void StandbyServiceImpl::GetAllowListInner(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
    uint32_t reasonCode)
{
    STANDBYSERVICE_LOGD("start GetAllowListInner, allowType is %{public}d", allowType);

    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    for (uint32_t allowTypeIndex = 0; allowTypeIndex < MAX_ALLOW_TYPE_NUM; ++allowTypeIndex) {
        uint32_t allowNumber = allowType & (1 << allowTypeIndex);
        if (allowNumber == 0) {
            continue;
        }
        GetTemporaryAllowList(allowTypeIndex, allowInfoList, reasonCode);
        bool isApp = (reasonCode == ReasonCodeEnum::REASON_APP_API);
        GetPersistAllowList(allowTypeIndex, allowInfoList, true, isApp);
    }
}

void StandbyServiceImpl::GetTemporaryAllowList(uint32_t allowTypeIndex, std::vector<AllowInfo>&
    allowInfoList, uint32_t reasonCode)
{
    int32_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    auto findRecordTask = [allowTypeIndex](const auto& it) { return it.allowTypeIndex_ == allowTypeIndex; };
    for (auto& [key, allowRecordPtr] : allowInfoMap_) {
        if ((allowRecordPtr->allowType_ & (1 << allowTypeIndex)) == 0) {
            continue;
        }
        if (allowRecordPtr->reasonCode_ != reasonCode) {
            continue;
        }
        auto& allowTimeList = allowRecordPtr->allowTimeList_;
        auto it = std::find_if(allowTimeList.begin(), allowTimeList.end(), findRecordTask);
        if (it == allowTimeList.end()) {
            continue;
        }
        allowInfoList.emplace_back((1 << allowTypeIndex), allowRecordPtr->name_,
            std::max(static_cast<long>(it->endTime_ - curTime), 0L));
    }
}

void StandbyServiceImpl::GetPersistAllowList(uint32_t allowTypeIndex, std::vector<AllowInfo>& allowInfoList,
    bool isAllow, bool isApp)
{
    uint32_t condition = TimeProvider::GetCondition();
    std::set<std::string> psersistAllowList;
    if (isApp) {
        psersistAllowList = StandbyConfigManager::GetInstance()->GetEligiblePersistAllowConfig(
            AllowTypeName[allowTypeIndex], condition, isAllow, true);
    } else {
        psersistAllowList = StandbyConfigManager::GetInstance()->GetEligiblePersistAllowConfig(
            AllowTypeName[allowTypeIndex], condition, isAllow, false);
    }
    for (const auto& allowName : psersistAllowList) {
        allowInfoList.emplace_back((1 << allowTypeIndex), allowName, -1);
    }
}

ErrCode StandbyServiceImpl::IsDeviceInStandby(bool& isStandby)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    handler_->PostSyncTask([this, &isStandby]() {
        auto curState = standbyStateManager_->GetCurState();
        isStandby = (curState == StandbyState::SLEEP);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::GetEligiableRestrictSet(uint32_t allowType, const std::string& strategyName,
    uint32_t resonCode, std::set<std::string>& restrictSet)
{
    uint32_t condition = TimeProvider::GetCondition();
    std::set<std::string> originRestrictSet = StandbyConfigManager::GetInstance()->GetEligiblePersistAllowConfig(
        strategyName, condition, false, resonCode == ReasonCodeEnum::REASON_APP_API);
    std::vector<AllowInfo> allowInfoList;
    GetAllowListInner(allowType, allowInfoList, resonCode);
    std::set<std::string> allowSet;
    for_each(allowInfoList.begin(), allowInfoList.end(),
        [&allowSet](AllowInfo& allowInfo) { allowSet.insert(allowInfo.GetName()); });

    std::set_difference(originRestrictSet.begin(), originRestrictSet.end(), allowSet.begin(),
        allowSet.end(), std::inserter(restrictSet, restrictSet.begin()));
    STANDBYSERVICE_LOGD("origin restrict size is %{public}d, restrictSet size is %{public}d, "\
        "restrictSet size is %{public}d", static_cast<int32_t>(originRestrictSet.size()),
        static_cast<int32_t>(allowInfoList.size()), static_cast<int32_t>(restrictSet.size()));
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName)
{
    if (!isServiceReady_.load()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("work scheduler status changed, isstarted: %{public}d, uid: %{public}d, bundleName: %{public}s",
        started, uid, bundleName.c_str());
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    if (CheckNativePermission(tokenId) != ERR_OK) {
        STANDBYSERVICE_LOGW("invoker is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    StandbyMessage standbyMessage {StandbyMessageType::BG_TASK_STATUS_CHANGE};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam(BG_TASK_TYPE, WORK_SCHEDULER);
    standbyMessage.want_->SetParam(BG_TASK_STATUS, started);
    standbyMessage.want_->SetParam(BG_TASK_UID, uid);
    DispatchEvent(standbyMessage);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
    uint32_t reasonCode)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start GetRestrictList");
    if (CheckNativePermission(OHOS::IPCSkeleton::GetCallingTokenID()) != ERR_OK) {
        STANDBYSERVICE_LOGW("invoker is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    if (!CheckAllowTypeInfo(restrictType)) {
        STANDBYSERVICE_LOGE("restrictType param is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    GetRestrictListInner(restrictType, restrictInfoList, reasonCode);
    return ERR_OK;
}

void StandbyServiceImpl::GetRestrictListInner(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
    uint32_t reasonCode)
{
    STANDBYSERVICE_LOGD("start GetRestrictListInner, restrictType is %{public}d", restrictType);
    for (uint32_t restrictTypeIndex = 0; restrictTypeIndex < MAX_ALLOW_TYPE_NUM; ++restrictTypeIndex) {
        uint32_t restrictNumber = restrictType & (1 << restrictTypeIndex);
        if (restrictNumber == 0) {
            continue;
        }
        bool isApp = (reasonCode == ReasonCodeEnum::REASON_APP_API);
        GetPersistAllowList(restrictTypeIndex, restrictInfoList, false, isApp);
    }
}

ErrCode StandbyServiceImpl::IsStrategyEnabled(const std::string& strategyName, bool& isStandby)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGD("standby service is not ready");
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start IsStrategyEnabled");
    if (CheckNativePermission(OHOS::IPCSkeleton::GetCallingTokenID()) != ERR_OK) {
        STANDBYSERVICE_LOGW("invoker is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    const auto& strategyConfigList = StandbyConfigManager::GetInstance()->GetStrategyConfigList();
    auto item = std::find(strategyConfigList.begin(), strategyConfigList.end(), strategyName);
    isStandby = item != strategyConfigList.end();
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ReportDeviceStateChanged(DeviceStateType type, bool enabled)
{
    if (!isServiceReady_.load()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGI("device state changed, state type: %{public}d, enabled: %{public}d",
        static_cast<int32_t>(type), enabled);
    if (CheckNativePermission(OHOS::IPCSkeleton::GetCallingTokenID()) != ERR_OK) {
        STANDBYSERVICE_LOGE("dump user is unpermitted due to not native process or shell");
        return ERR_STANDBY_PERMISSION_DENIED;
    }
    DeviceStateCache::GetInstance()->SetDeviceState(static_cast<int32_t>(type), enabled);
    if (!enabled) {
        return ERR_OK;
    }
    StandbyMessage standbyMessage {StandbyMessageType::DEVICE_STATE_CHANGED};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("DIS_COMP_STATE", enabled);
    DispatchEvent(standbyMessage);
    return ERR_OK;
}

void WEAK_FUNC StandbyServiceImpl::HandleCallStateChanged(const std::string &sceneInfo)
{
    nlohmann::json payload = nlohmann::json::parse(sceneInfo, nullptr, false);
    if (payload.is_discarded()) {
        STANDBYSERVICE_LOGE("parse json failed");
    }
    int32_t state = -1;
    if (payload.at("state").is_string()) {
        state = atoi(payload["state"].get<std::string>().c_str());
    }
    if (payload.at("state").is_number_integer()) {
        state = payload["state"].get<std::int32_t>();
    }
    bool disable = (state == static_cast<int32_t>(TelCallState::CALL_STATUS_UNKNOWN) ||
                    state == static_cast<int32_t>(TelCallState::CALL_STATUS_DISCONNECTED) ||
                    state == static_cast<int32_t>(TelCallState::CALL_STATUS_IDLE));
    DeviceStateCache::GetInstance()->SetDeviceState(
    static_cast<int32_t>(DeviceStateType::TELEPHONE_STATE_CHANGE), !disable);
}

void WEAK_FUNC StandbyServiceImpl::HandleP2PStateChanged(int32_t state)
{
    bool disable = (state == static_cast<int32_t>(P2pState::P2P_STATE_IDLE) ||
                    state == static_cast<int32_t>(P2pState::P2P_STATE_NONE) ||
                    state == static_cast<int32_t>(P2pState::P2P_STATE_CLOSED));
    DeviceStateCache::GetInstance()->SetDeviceState(
    static_cast<int32_t>(DeviceStateType::WIFI_P2P_CHANGE), !disable);
}

void StandbyServiceImpl::HandleScreenStateChanged(const int64_t value)
{
    if (value == 1) {
            DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                         EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON));
    } else {
            DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                         EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF));
    }
}

void StandbyServiceImpl::HandleResourcesStateChanged(const int64_t value, const std::string &sceneInfo)
{
        bool isApply = false;
        if (value == ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY ||
            value == ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY) {
            isApply = true;
        }
        nlohmann::json payload = nlohmann::json::parse(sceneInfo, nullptr, false);
        if (payload.is_discarded()) {
            STANDBYSERVICE_LOGE("parse json failed");
            return;
        }
        if (!payload.contains("bundleName") || !payload.contains("resourceNumber")) {
            STANDBYSERVICE_LOGE("param does not exist");
            return;
        }
        if (!payload.at("bundleName").is_string()) {
            STANDBYSERVICE_LOGE("bundle name param is invalid");
            return;
        }
        std::string bundleName = payload.at("bundleName").get<std::string>();
        if (!payload.at("resourceNumber").is_number_unsigned()) {
            STANDBYSERVICE_LOGE("resource number param is invalid");
            return;
        }
        uint32_t resourceNumber = payload["resourceNumber"].get<std::uint32_t>();
        StandbyMessage standbyMessage {StandbyMessageType::BG_EFFICIENCY_RESOURCE_APPLY};
        standbyMessage.want_ = AAFwk::Want {};
        standbyMessage.want_->SetParam(BG_TASK_BUNDLE_NAME, bundleName);
        standbyMessage.want_->SetParam(BG_TASK_RESOURCE_STATUS, isApply);
        standbyMessage.want_->SetParam(BG_TASK_TYPE, static_cast<int32_t>(resourceNumber));
        DispatchEvent(standbyMessage);
}

ErrCode StandbyServiceImpl::HandleCommonEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo)
{
    STANDBYSERVICE_LOGI("HandleCommonEvent resType = %{public}u, value = %{public}lld, sceneInfo = %{public}s",
                        resType, (long long)(value), sceneInfo.c_str());
    switch (resType) {
        case ResType::RES_TYPE_SCREEN_STATUS:
            HandleScreenStateChanged(value);
            break;
        case ResType::RES_TYPE_CHARGING_DISCHARGING:
            if (value == 0) {
                DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                             EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING));
            } else {
                DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                             EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING));
            }
            break;
        case ResType::RES_TYPE_USB_DEVICE:
            if (value == 0) {
                DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                             EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED));
            } else {
                DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                                             EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED));
            }
            break;
        case ResType::RES_TYPE_CALL_STATE_CHANGED:
            HandleCallStateChanged(sceneInfo);
            break;
        case ResType::RES_TYPE_WIFI_P2P_STATE_CHANGED:
            HandleP2PStateChanged(value);
            break;
#ifdef STANDBY_POWER_MANAGER_ENABLE
        case ResType::RES_TYPE_POWER_MODE_CHANGED:
            HandlePowerModeChanged(static_cast<PowerMgr::PowerMode>(value));
            break;
#endif
        case ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED:
            HandleResourcesStateChanged(value, sceneInfo);
            break;
        default:
            AppEventHandler(resType, value, sceneInfo);
            break;
    }
    return ERR_OK;
}

#ifdef STANDBY_POWER_MANAGER_ENABLE
void StandbyServiceImpl::HandlePowerModeChanged(PowerMgr::PowerMode mode)
{
    bool isSaveMode = false;
    if (mode == PowerMgr::PowerMode::POWER_SAVE_MODE || mode == PowerMgr::PowerMode::EXTREME_POWER_SAVE_MODE) {
        isSaveMode = true;
    }
    
    StandbyMessage message(StandbyMessageType::COMMON_EVENT);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
    message.want_ = AAFwk::Want {};
    message.want_->SetParam("current_power_mode", isSaveMode);
    DispatchEvent(message);
}
#endif

void StandbyServiceImpl::AppEventHandler(const uint32_t resType, const int64_t value, const std::string &sceneInfo)
{
    if (resType == ResType::RES_TYPE_APP_INSTALL_UNINSTALL &&
        (value == ResType::AppInstallStatus::APP_UNINSTALL ||
         value == ResType::AppInstallStatus::APP_CHANGED ||
         value == ResType::AppInstallStatus::APP_REPLACED ||
         value == ResType::AppInstallStatus::BUNDLE_REMOVED ||
         value == ResType::AppInstallStatus::APP_FULLY_REMOVED)
        ) {
        nlohmann::json payload = nlohmann::json::parse(sceneInfo, nullptr, false);
        if (payload.is_discarded()) {
            STANDBYSERVICE_LOGE("parse json failed");
            return;
        }
        if (!payload.contains("bundleName") || !payload.contains("uid")) {
            STANDBYSERVICE_LOGE("HandleCommonEvent,There is no valid bundle name in payload");
            return;
        }
        if (!payload.at("bundleName").is_string()) {
            STANDBYSERVICE_LOGE("bundle name is invaild");
            return;
        }
        std::string bundleName = payload.at("bundleName").get<std::string>();
        int32_t uid = -1;
        if (payload.at("uid").is_string()) {
            uid = atoi(payload["uid"].get<std::string>().c_str());
        }
        if (payload.at("uid").is_number_integer()) {
            uid = payload["uid"].get<std::int32_t>();
        }
        handler_->PostTask([uid, bundleName]() {
            StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(uid, bundleName, true);
        });
    } else if (resType == ResType::RES_TYPE_TIMEZONE_CHANGED ||
               resType == ResType::RES_TYPE_NITZ_TIMEZONE_CHANGED ||
               resType == ResType::RES_TYPE_TIME_CHANGED ||
               resType == ResType::RES_TYPE_NITZ_TIME_CHANGED) {
        handler_->PostTask([]() {StandbyServiceImpl::GetInstance()->ResetTimeObserver(); });
    }
}

void StandbyServiceImpl::DispatchEvent(const StandbyMessage& message)
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGW("standby service is not ready");
        return;
    }

    auto dispatchEventFunc = [this, message]() {
        STANDBYSERVICE_LOGD("standby service implement dispatch message %{public}d", message.eventId_);
        if (!listenerManager_ || !standbyStateManager_ || !strategyManager_) {
            STANDBYSERVICE_LOGE("can not dispatch event, state manager or strategy manager is nullptr");
            return;
        };
        listenerManager_->HandleEvent(message);
        standbyStateManager_->HandleEvent(message);
        strategyManager_->HandleEvent(message);
    };

    handler_->PostTask(dispatchEventFunc);
}

bool StandbyServiceImpl::IsDebugMode()
{
    return debugMode_;
}

void StandbyServiceImpl::ShellDump(const std::vector<std::string>& argsInStr,
    std::string& result)
{
    if (!isServiceReady_.load()) {
        result += "standby service manager is not ready";
        return;
    }
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    if (CheckNativePermission(tokenId) != ERR_OK) {
        STANDBYSERVICE_LOGE("dump user is unpermitted due to not native process or shell");
        result += "please using root identity\n";
        return;
    }
    handler_->PostSyncTask([this, &argsInStr, &result]() {
        this->ShellDumpInner(argsInStr, result);
        }, AppExecFwk::EventQueue::Priority::HIGH);
}

void StandbyServiceImpl::ShellDumpInner(const std::vector<std::string>& argsInStr,
    std::string& result)
{
    auto argc = argsInStr.size();
    if (argc == NO_DUMP_PARAM_NUMS || argsInStr[DUMP_FIRST_PARAM] == "-h") {
        DumpUsage(result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_DETAIL_INFO) {
        DumpShowDetailInfo(argsInStr, result);
        OnPluginShellDump(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_ENTER_STATE) {
        DumpEnterSpecifiedState(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_APPLY_ALLOW_RECORD) {
        DumpModifyAllowList(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_SIMULATE_SENSOR) {
        OnPluginShellDump(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_SUBSCRIBER_OBSERVER) {
        DumpSubScriberObserver(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_TURN_ON_OFF_SWITCH) {
        DumpTurnOnOffSwitch(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_CHANGE_STATE_TIMEOUT) {
        DumpChangeConfigParam(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_PUSH_STRATEGY_CHANGE) {
        DumpPushStrategyChange(argsInStr, result);
    } else {
        result += "Error params.\n";
    }
}

void StandbyServiceImpl::OnPluginShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    standbyStateManager_->ShellDump(argsInStr, result);
    constraintManager_->ShellDump(argsInStr, result);
    strategyManager_->ShellDump(argsInStr, result);
    listenerManager_->ShellDump(argsInStr, result);
}

void StandbyServiceImpl::DumpUsage(std::string& result)
{
    std::string dumpHelpMsg =
    "usage: dev standby service dump [<options>]\n"
    "options list:\n"
    "    -h                                                 help menu\n"
    "    -D                                                 show detail information\n"
    "        --config                                            show all info, including config\n"
    "        --reset_state                                       reset parameter, validate debug parameter\n"
    "        --strategy                                          dump strategy info\n"
    "    -E                                                 enter the specified state:\n"
    "        {name of state} {whether skip evalution}       enter the specified state, respectively named\n"
    "                                                            woking, dark, nap, maintenance, sleep\n"
    "    -A                                                 modify the allow list:\n"
    "        --apply {uid} {name} {type} {duration} {reasoncode} apply the type of the uid to allow list\n"
    "        --unapply {uid} {name} {type}                  delete the type of the uid from allow list\n"
    "        --get {type} {isApp}                                get allow list info\n"
    "    -S                                                 simulate some action:\n"
    "        {--motion}                                          activate the motion sensor when enter idle\n"
    "        {--repeat}                                          be in motion mode, only used in idle state\n"
    "        {--blocked}                                         block current state\n"
    "        {--poweroff}                                        power off strategy\n"
    "        {--powersave}                                       enable power save firwwall\n"
    "        {--halfhour}                                        screen off for half hour\n"
    "    -T  {switch name} {on or off}                      turn on or turn off some switches, switch can be debug,\n"
    "                                                            nap_switch, sleep_switch, detect_motion, other\n"
    "                                                            switch only be used after open debug switch\n"
    "    -C  {parameter name} {parameter value}             change config parameter, only can be used when debug\n"
    "    -P                                                 sending network limiting and restoring network broadcasts\n"
    "        {--whitelist} {parameter value}                send whitelist changes event\n"
    "        {--ctrinetwork}                                send network limiting broadcasts\n"
    "        {--restorectrlnetwork}                         send restore network broadcasts\n";

    result.append(dumpHelpMsg);
}

void StandbyServiceImpl::DumpShowDetailInfo(const std::vector<std::string>& argsInStr,
    std::string& result)
{
    DumpAllowListInfo(result);
    if (argsInStr.size() < DUMP_DETAILED_INFO_MAX_NUMS) {
        return;
    }
    if (argsInStr[DUMP_SECOND_PARAM] == DUMP_DETAIL_CONFIG) {
        DumpStandbyConfigInfo(result);
    }
}

void StandbyServiceImpl::DumpAllowListInfo(std::string& result)
{
    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    if (allowInfoMap_.empty()) {
        result += "allow resources record is empty\n";
        return;
    }

    std::stringstream stream;
    uint32_t index = 1;
    for (auto iter = allowInfoMap_.begin(); iter != allowInfoMap_.end(); iter++) {
        stream << "No." << index << "\n";
        stream << "\tuid: " << iter->first << "\n";
        stream << "\tallow record: " << "\n";
        stream << "\t\tname: " << iter->second->name_ << "\n";
        stream << "\t\tpid: " << iter->second->pid_ << "\n";
        stream << "\t\tallow type: " << iter->second->allowType_ << "\n";
        stream << "\t\treason code: " << iter->second->reasonCode_ << "\n";
        int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
        auto &allowTimeList = iter->second->allowTimeList_;
        for (auto unitIter = allowTimeList.begin();
            unitIter != allowTimeList.end(); ++unitIter) {
            stream << "\t\t\tallow type: " << AllowTypeName[unitIter->allowTypeIndex_] << "\n";
            stream << "\t\t\tremainTime: " << unitIter->endTime_ - curTime << "\n";
            stream << "\t\t\treason: " << unitIter->reason_ << "\n";
        }
        stream << "\n";
        result += stream.str();
        stream.str("");
        stream.clear();
        index++;
    }
}

void StandbyServiceImpl::DumpStandbyConfigInfo(std::string& result)
{
    result += (debugMode_ ? "debugMode: true\n" : "debugMode: false\n");
    StandbyConfigManager::GetInstance()->DumpStandbyConfigInfo(result);
}

void StandbyServiceImpl::DumpEnterSpecifiedState(const std::vector<std::string>& argsInStr,
    std::string& result)
{
    if (argsInStr.size() < DUMP_SLEEP_ENTER_STATE_NUMS) {
        result += "not enough parameter for changing sleep mode\n";
        return;
    } else {
        standbyStateManager_->ShellDump(argsInStr, result);
    }
}

void StandbyServiceImpl::DumpModifyAllowList(const std::vector<std::string>& argsInStr,
    std::string& result)
{
    if (argsInStr.size() < DUMP_SLEEP_ALLOW_LIST_NUMS || (argsInStr[DUMP_SECOND_PARAM] != "--get" &&
        argsInStr.size() < DUMP_SLEEP_APPLY_ALLOW_LIST_NUMS)) {
        result += "not enough parameter for changing allow list\n";
        return;
    }
    int32_t uid = std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str());
    std::string name = argsInStr[DUMP_FOURTH_PARAM];
    if (argsInStr[DUMP_SECOND_PARAM] == "--apply") {
        uint32_t allowType = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_FIFTH_PARAM].c_str()));
        int32_t duration = std::atoi(argsInStr[DUMP_SIXTH_PARAM].c_str());
        sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest(allowType,
            uid, name, duration, "dump", std::atoi(argsInStr[DUMP_SEVENTH_PARAM].c_str()));
        ApplyAllowResource(resourceRequest);
        result += "add one object to allow list\n";
    } else if (argsInStr[DUMP_SECOND_PARAM] == "--unapply") {
        uint32_t allowType = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_FIFTH_PARAM].c_str()));
        sptr<ResourceRequest> resourceRequest = new (std::nothrow) ResourceRequest(allowType,
            uid, name, 0, "dump", std::atoi(argsInStr[DUMP_SEVENTH_PARAM].c_str()));
        UnapplyAllowResource(resourceRequest);
        result += "remove one object to allow list\n";
    } else if (argsInStr[DUMP_SECOND_PARAM] == "--get") {
        uint32_t allowType = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str()));
        bool isApp = (std::atoi(argsInStr[DUMP_FOURTH_PARAM].c_str()) == 0);
        std::vector<AllowInfo> allowInfoList;
        GetAllowListInner(allowType, allowInfoList, isApp);
        for (const auto& allowInfo : allowInfoList) {
            result += "allowType: " + std::to_string(allowInfo.GetAllowType()) + "\n" +
            "name: " + allowInfo.GetName() + "\n" +
            "duration: " + std::to_string(allowInfo.GetDuration()) + "\n";
        }
        allowInfoList.clear();
        GetRestrictListInner(allowType, allowInfoList, isApp);
        for (const auto& allowInfo : allowInfoList) {
            result += "restrictType: " + std::to_string(allowInfo.GetAllowType()) + "\n" +
            "name: " + allowInfo.GetName() + "\n";
        }
    }
}

void StandbyServiceImpl::DumpTurnOnOffSwitch(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr.size() != DUMP_SWITCH_PARAM_NUMS) {
        result += "not correct parameter number for turn on or turn off switch\n";
        return;
    }
    bool switchStatus {false};
    if (argsInStr[DUMP_THIRD_PARAM] == DUMP_ON) {
        switchStatus = true;
    } else if (argsInStr[DUMP_THIRD_PARAM] == DUMP_OFF) {
        switchStatus = false;
    } else {
        result += "not correct parameter for turn on or turn off switch\n";
        return;
    }
    const std::string& switchName = argsInStr[DUMP_SECOND_PARAM];
    if (switchName == DUMP_DEBUG_SWITCH) {
        debugMode_ = switchStatus;
        StandbyConfigManager::GetInstance()->DumpSetDebugMode(debugMode_);
        result += (debugMode_ ? "debugMode: true\n" : "debugMode: false\n");
        return;
    } else if (!debugMode_) {
        result += "other switch can be changed only in debug mode\n";
        return;
    }
    StandbyConfigManager::GetInstance()->DumpSetSwitch(switchName, switchStatus, result);
}

void StandbyServiceImpl::DumpChangeConfigParam(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr.size() != DUMP_STATE_TIMEOUT_PARAM_NUMS) {
        result += "not correct parameter number for change state timeout\n";
        return;
    }
    if (!debugMode_) {
        result += "current is not in debug mode, can not change timeout\n";
        return;
    }
    StandbyConfigManager::GetInstance()->DumpSetParameter(argsInStr[DUMP_SECOND_PARAM],
        std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str()), result);
}

void StandbyServiceImpl::DumpPushStrategyChange(const std::vector<std::string>& argsInStr, std::string& result)
{
    if (argsInStr[DUMP_SECOND_PARAM] == "--whitelist") {
        StandbyStateSubscriber::GetInstance()->NotifyAllowChangedByCommonEvent(
            std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str()), argsInStr[DUMP_FOURTH_PARAM],
            std::atoi(argsInStr[DUMP_FIFTH_PARAM].c_str()), argsInStr[DUMP_SIXTH_PARAM] == "true");
    }
    strategyManager_->ShellDump(argsInStr, result);
}

void StandbyServiceImpl::DumpSubScriberObserver(const std::vector<std::string>& argsInStr, std::string& result)
{
    StandbyStateSubscriber::GetInstance()->ShellDump(argsInStr, result);
}

IMPLEMENT_SINGLE_INSTANCE(DeviceStateCache);

DeviceStateCache::DeviceStateCache()
{
    deviceState_ = {false, false, false};
}

DeviceStateCache::~DeviceStateCache() {}

bool DeviceStateCache::SetDeviceState(int32_t type, bool enabled)
{
    STANDBYSERVICE_LOGD("set device state %{public}d, enabled is %{public}d", type, enabled);
    if (type < 0 || type >= DEVICE_STATE_NUM) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (deviceState_[type] == enabled) {
        return false;
    }
    deviceState_[type] = enabled;
    return true;
}

bool DeviceStateCache::GetDeviceState(int32_t type)
{
    if (type < 0 || type >= DEVICE_STATE_NUM) {
        return false;
    }
    STANDBYSERVICE_LOGD("get device state %{public}d, enabled is %{public}d", type, deviceState_[type]);
    return deviceState_[type];
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
