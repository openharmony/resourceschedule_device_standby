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
#include <fcntl.h>
#include <file_ex.h>
#include <functional>
#include <securec.h>
#include <set>
#include <sstream>
#include <string>
#include <unique_fd.h>
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
#include "res_common_util.h"
#include "res_sched_event_reporter.h"
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
const std::string CLONE_BACKUP_FILE_PATH = "/data/service/el1/public/device_standby/device_standby_clone";
const std::string DEVICE_STANDBY_DIR = "/data/service/el1/public/device_standby";
const std::string DEVICE_STANDBY_RDB_DIR = "/data/service/el3/100/device_standby/rdb";
const std::string STANDBY_MSG_HANDLER = "StandbyMsgHandler";
const std::string ON_PLUGIN_REGISTER = "OnPluginRegister";
const std::string STANDBY_EXEMPTION_PERMISSION = "ohos.permission.DEVICE_STANDBY_EXEMPTION";
const uint32_t EXEMPT_ALL_RESOURCES = 100;
const std::string COMMON_EVENT_TIMER_SA_ABILITY = "COMMON_EVENT_TIMER_SA_ABILITY";
const uint32_t ONE_SECOND = 1000;
const std::string DUMP_ON_POWER_OVERUSED = "--poweroverused";
const std::string DUMP_ON_ACTION_CHANGED = "--actionchanged";
const int32_t EXTENSION_ERROR_CODE = 13500099;
}

StandbyServiceImpl::StandbyServiceImpl() {}

StandbyServiceImpl::~StandbyServiceImpl() {}

std::shared_ptr<StandbyServiceImpl> StandbyServiceImpl::GetInstance()
{
    return DelayedSingleton<StandbyServiceImpl>::GetInstance();
}

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
    HandleReportFileSizeEvent();
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
            STANDBYSERVICE_LOGE("Dispatch COMMON_EVENT_TIMER_SA_ABILITY begin");
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

void StandbyServiceImpl::UpdateSaDependValue(const bool& isAdd, const uint32_t& saId)
{
    if (isAdd) {
        dependsReady_ |= saId;
    } else {
        dependsReady_ &= (~saId);
    }
}

uint32_t StandbyServiceImpl::GetSaDependValue()
{
    return dependsReady_;
}

bool StandbyServiceImpl::IsServiceReady()
{
    if (!isServiceReady_.load()) {
        STANDBYSERVICE_LOGW("standby service is not ready, dependsReady is %{public}d", dependsReady_);
        return false;
    }
    return true;
}

ErrCode StandbyServiceImpl::RegisterPlugin(const std::string& pluginName)
{
    STANDBYSERVICE_LOGI("start register plugin %{public}s", pluginName.c_str());
    registerPlugin_ = dlopen(pluginName.c_str(), RTLD_NOW);
    if (!registerPlugin_) {
        dlclose(registerPlugin_);
        STANDBYSERVICE_LOGE("failed to open plugin %{public}s", pluginName.c_str());
        return ERR_STANDBY_PLUGIN_NOT_EXIST;
    }
    void* pluginFunc = dlsym(registerPlugin_, ON_PLUGIN_REGISTER.c_str());
    if (!pluginFunc) {
        dlclose(registerPlugin_);
        STANDBYSERVICE_LOGE("failed to find extern func of plugin %{public}s", pluginName.c_str());
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

ErrCode StandbyServiceImpl::SubscribeBackupRestoreCallback(const std::string& moduleName,
    const std::function<ErrCode(std::vector<char>&)>& onBackupFunc,
    const std::function<ErrCode(std::vector<char>&)>& onRestoreFunc)
{
    std::lock_guard<std::mutex> lock(backupRestoreMutex_);
    if (onBackupFuncMap_.find(moduleName) != onBackupFuncMap_.end()) {
        STANDBYSERVICE_LOGE("Repeat subscribe backup restore callback, module name: %{public}s", moduleName.c_str());
        return ERR_INVALID_OPERATION;
    }

    onBackupFuncMap_.insert(std::make_pair(moduleName, onBackupFunc));
    onRestoreFuncMap_.insert(std::make_pair(moduleName, onRestoreFunc));
    return ERR_OK;
}

ErrCode StandbyServiceImpl::UnsubscribeBackupRestoreCallback(const std::string& moduleName)
{
    std::lock_guard<std::mutex> lock(backupRestoreMutex_);
    onBackupFuncMap_.erase(moduleName);
    onRestoreFuncMap_.erase(moduleName);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::OnBackup(MessageParcel& data, MessageParcel& reply)
{
    UniqueFd fd(-1);
    std::string replyCode = BuildBackupReplyCode(0);
    std::vector<char> buff;
    {
        std::lock_guard<std::mutex> lock(backupRestoreMutex_);
        for (const auto& [moduleName, func] : onBackupFuncMap_) {
            std::vector<char> moduleBuff;
            CloneFileHead head {};
            ErrCode err = func(moduleBuff);
            if (err != ERR_OK ||
                strncpy_s(head.moduleName, sizeof(head.moduleName), moduleName.c_str(), moduleName.size()) != 0) {
                continue;
            }
            head.fileOffset = buff.size();
            head.fileSize = moduleBuff.size();
            std::copy(reinterpret_cast<char*>(&head), reinterpret_cast<char*>(&head) + sizeof(CloneFileHead),
                std::back_inserter(buff));
            buff.insert(buff.end(), moduleBuff.begin(), moduleBuff.end());
        }
    }
    if (buff.size() != 0 && SaveBufferToFile(CLONE_BACKUP_FILE_PATH, buff)) {
        fd = UniqueFd(open(CLONE_BACKUP_FILE_PATH.c_str(), O_RDONLY));
    } else {
        STANDBYSERVICE_LOGE("OnBackup fail: save buff to file fail");
        replyCode  = BuildBackupReplyCode(EXTENSION_ERROR_CODE);
    }

    if ((!reply.WriteFileDescriptor(fd)) || (!reply.WriteString(replyCode))) {
        close(fd.Release());
        int32_t ret = remove(CLONE_BACKUP_FILE_PATH.c_str());
        STANDBYSERVICE_LOGE("OnBackup fail: reply write fail! remove ret: %{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    close(fd.Release());
    int32_t ret = remove(CLONE_BACKUP_FILE_PATH.c_str());
    STANDBYSERVICE_LOGI("OnBackup succ: backup data success! remove ret: %{public}d", ret);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::OnRestore(MessageParcel& data, MessageParcel& reply)
{
    std::string replyCode = BuildBackupReplyCode(0);
    std::vector<char> buff;
    UniqueFd fd(data.ReadFileDescriptor());
    if (fd.Get() < 0) {
        STANDBYSERVICE_LOGE("OnRestore fail: ReadFileDescriptor fail");
        return ERR_INVALID_OPERATION;
    }
    const off_t fileLength = lseek(fd.Get(), 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    buff.resize(fileLength);
    const ssize_t len = read(fd, buff.data(), fileLength);
    if (len == fileLength && fileLength != 0) {
        std::lock_guard<std::mutex> lock(backupRestoreMutex_);
        off_t offset = 0;
        while (offset + static_cast<off_t>(sizeof(CloneFileHead)) < fileLength) {
            CloneFileHead* head = reinterpret_cast<CloneFileHead*>(buff.data() + offset);
            head->moduleName[sizeof(CloneFileHead::moduleName) - 1] = '\0';
            if (offset + static_cast<off_t>(sizeof(CloneFileHead) + head->fileSize) > fileLength) {
                break;
            }
            const auto& onRestoreFunc = onRestoreFuncMap_.find(std::string(head->moduleName));
            if (onRestoreFunc != onRestoreFuncMap_.end()) {
                std::vector<char> moduleBuff;
                std::copy(reinterpret_cast<char*>(&head->data), reinterpret_cast<char*>(&head->data) + head->fileSize,
                    std::back_inserter(moduleBuff));
                (onRestoreFunc->second)(moduleBuff);
            }
            offset += static_cast<off_t>(sizeof(CloneFileHead) + head->fileSize);
        }
    } else {
        STANDBYSERVICE_LOGE("OnRestore fail: get file fail");
        replyCode  = BuildBackupReplyCode(EXTENSION_ERROR_CODE);
    }

    if (!reply.WriteString(replyCode)) {
        close(fd.Release());
        STANDBYSERVICE_LOGE("OnRestore fail: reply write fail!");
        return ERR_INVALID_OPERATION;
    }
    close(fd.Release());
    STANDBYSERVICE_LOGI("OnRestore succ: reply write success!");
    return ERR_OK;
}

std::string StandbyServiceImpl::BuildBackupReplyCode(int32_t replyCode)
{
    nlohmann::json root;
    nlohmann::json resultInfo;
    nlohmann::json errorInfo;

    errorInfo["type"] = "ErrorInfo";
    errorInfo["errorCode"] = std::to_string(replyCode);
    errorInfo["errorInfo"] = "";

    resultInfo.push_back(errorInfo);
    root["resultInfo"] = resultInfo;
    return root.dump();
}

bool StandbyServiceImpl::CheckAllowTypeInfo(uint32_t allowType)
{
    return allowType > 0 && allowType <= MAX_ALLOW_TYPE_NUMBER;
}

ErrCode StandbyServiceImpl::RemoveAppAllowRecord(int32_t uid, const std::string &bundleName, bool resetAll)
{
    if (!IsServiceReady()) {
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
    return ERR_OK;
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
    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    STANDBYSERVICE_LOGI("add %{public}s subscriber to stanby service", subscriber->GetSubscriberName().c_str());
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
    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    STANDBYSERVICE_LOGI("add subscriber to stanby service succeed");
    return StandbyStateSubscriber::GetInstance()->RemoveSubscriber(subscriber);
}

ErrCode StandbyServiceImpl::ApplyAllowResource(ResourceRequest& resourceRequest)
{
    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start AddAllowList");
    if (auto checkRet = CheckCallerPermission(resourceRequest.GetReasonCode()); checkRet != ERR_OK) {
        return checkRet;
    }

    // update allow type according to configuration
    if (Security::AccessToken::AccessTokenKit::GetTokenType(OHOS::IPCSkeleton::GetCallingTokenID())
        == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        resourceRequest.SetAllowType(GetExemptedResourceType(resourceRequest.GetAllowType()));
    }

    if (!CheckAllowTypeInfo(resourceRequest.GetAllowType()) || resourceRequest.GetUid() < 0) {
        STANDBYSERVICE_LOGE("resourceRequest param is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    if (resourceRequest.GetDuration() < 0) {
        STANDBYSERVICE_LOGE("duration param is invalid");
        return ERR_DURATION_INVALID;
    }
    int32_t pid = IPCSkeleton::GetCallingPid();
    ApplyAllowResInner(resourceRequest, pid);
    return ERR_OK;
}

void StandbyServiceImpl::ApplyAllowResInner(const ResourceRequest& resourceRequest, int32_t pid)
{
    STANDBYSERVICE_LOGI("apply res inner, uid: %{public}d, name: %{public}s, allowType: %{public}u,"\
        " duration: %{public}d, reason: %{public}s", resourceRequest.GetUid(),
        resourceRequest.GetName().c_str(), resourceRequest.GetAllowType(),
        resourceRequest.GetDuration(), resourceRequest.GetReason().c_str());

    int32_t uid = resourceRequest.GetUid();
    const std::string& name = resourceRequest.GetName();
    std::string keyStr = std::to_string(uid) + "_" + name;
    uint32_t preAllowType = 0;

    std::lock_guard<std::mutex> allowRecordLock(allowRecordMutex_);
    auto iter = allowInfoMap_.find(keyStr);
    if (iter == allowInfoMap_.end()) {
        std::tie(iter, std::ignore) =
            allowInfoMap_.emplace(keyStr, std::make_shared<AllowRecord>(uid, pid, name, 0));
        iter->second->reasonCode_ = resourceRequest.GetReasonCode();
    } else {
        preAllowType = iter->second->allowType_;
        iter->second->pid_ = pid;
    }
    UpdateRecord(iter->second, resourceRequest);
    if (preAllowType != iter->second->allowType_) {
        uint32_t alowTypeDiff = iter->second->allowType_ ^ (preAllowType &
            iter->second->allowType_);
        STANDBYSERVICE_LOGI("after update record, there is added exemption type: %{public}d",
            alowTypeDiff);
        StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(uid, name, alowTypeDiff, true);
        NotifyAllowListChanged(uid, name, alowTypeDiff, true);
    }
    if (iter->second->allowType_ == 0) {
        STANDBYSERVICE_LOGI("%{public}s does not have valid record, delete record", keyStr.c_str());
        allowInfoMap_.erase(iter);
    }
    DumpPersistantData();
}

void StandbyServiceImpl::UpdateRecord(std::shared_ptr<AllowRecord>& allowRecord,
    const ResourceRequest& resourceRequest)
{
    int32_t uid = resourceRequest.GetUid();
    const std::string& name = resourceRequest.GetName();
    uint32_t allowType = resourceRequest.GetAllowType();
    bool isApp = (resourceRequest.GetReasonCode() == ReasonCodeEnum::REASON_APP_API);
    int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    int64_t endTime {0};
    uint32_t condition = TimeProvider::GetCondition();
    STANDBYSERVICE_LOGI(
        "start UpdateRecord uid: %{public}d, name: %{public}s, isApp: %{public}d, condition: %{public}u",
        uid, name.c_str(), isApp, condition);
    for (uint32_t allowTypeIndex = 0; allowTypeIndex < MAX_ALLOW_TYPE_NUM; ++allowTypeIndex) {
        uint32_t allowNumber = allowType & (1 << allowTypeIndex);
        if (allowNumber == 0) {
            continue;
        }
        int64_t maxDuration = 0;
        if (allowNumber != AllowType::WORK_SCHEDULER) {
            maxDuration = std::min(resourceRequest.GetDuration(), StandbyConfigManager::GetInstance()->
                GetMaxDuration(name, AllowTypeName[allowTypeIndex], condition, isApp)) * TimeConstant::MSEC_PER_SEC;
        } else {
            maxDuration = resourceRequest.GetDuration() * TimeConstant::MSEC_PER_SEC;
        }
        if (maxDuration <= 0) {
            continue;
        }
        endTime = curTime + maxDuration;
        auto& allowTimeList = allowRecord->allowTimeList_;
        auto findRecordTask = [allowTypeIndex](const auto& it) { return it.allowTypeIndex_ == allowTypeIndex; };
        auto it = std::find_if(allowTimeList.begin(), allowTimeList.end(), findRecordTask);
        if (it == allowTimeList.end()) {
            allowTimeList.emplace_back(AllowTime {allowTypeIndex, endTime, resourceRequest.GetReason()});
        } else {
            it->reason_ = resourceRequest.GetReason();
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

ErrCode StandbyServiceImpl::UnapplyAllowResource(ResourceRequest& resourceRequest)
{
    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start UnapplyAllowResource");
    if (auto checkRet = CheckCallerPermission(resourceRequest.GetReasonCode()); checkRet != ERR_OK) {
        return checkRet;
    }

    // update allow type according to configuration
    if (Security::AccessToken::AccessTokenKit::GetTokenType(OHOS::IPCSkeleton::GetCallingTokenID())
        == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        resourceRequest.SetAllowType(GetExemptedResourceType(resourceRequest.GetAllowType()));
    }

    if (!CheckAllowTypeInfo(resourceRequest.GetAllowType()) || resourceRequest.GetUid() < 0) {
        STANDBYSERVICE_LOGE("param of resourceRequest is invalid");
        return ERR_RESOURCE_TYPES_INVALID;
    }
    UnapplyAllowResInner(resourceRequest.GetUid(), resourceRequest.GetName(), resourceRequest.GetAllowType(), true);
    return ERR_OK;
}

void StandbyServiceImpl::UnapplyAllowResInner(int32_t uid, const std::string& name,
    uint32_t allowType, bool removeAll)
{
    STANDBYSERVICE_LOGI("start UnapplyAllowResInner, uid is %{public}d, allowType is %{public}d, removeAll is "\
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
    int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
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
    } else {
        allowRecordPtr->allowType_ = allowRecordPtr->allowType_ - removedNumber;
    }
    StandbyStateSubscriber::GetInstance()->ReportAllowListChanged(uid, name, removedNumber, false);
    NotifyAllowListChanged(uid, name, removedNumber, false);
    DumpPersistantData();
}

void StandbyServiceImpl::OnProcessStatusChanged(int32_t uid, int32_t pid, const std::string& bundleName, bool isCreated)
{
    if (!IsServiceReady()) {
        return;
    }
    STANDBYSERVICE_LOGI("process status change, uid: %{public}d, pid: %{public}d, name: %{public}s, alive: %{public}d",
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
    if (!IsServiceReady()) {
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
    int64_t curTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
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
        int64_t duration =  std::max(static_cast<int64_t>(it->endTime_ - curTime), static_cast<int64_t>(0L));
        if (duration > 0) {
            allowInfoList.emplace_back((1 << allowTypeIndex), allowRecordPtr->name_, duration);
        } else {
            auto task = [this, allowRecordPtr = allowRecordPtr] () {
                this->UnapplyAllowResInner(allowRecordPtr->uid_, allowRecordPtr->name_,
                    allowRecordPtr->allowType_, false);
            };
            handler_->PostTask(task);
        }
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
    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    if (!IsServiceReady()) {
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
    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("work scheduler status changed, isstarted: %{public}d, uid: %{public}d, bundleName: %{public}s",
        started, uid, bundleName.c_str());
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
    if (auto checkRet = CheckCallerPermission(reasonCode); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }
    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start GetRestrictList");
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
    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    STANDBYSERVICE_LOGD("start IsStrategyEnabled");
    const auto& strategyConfigList = StandbyConfigManager::GetInstance()->GetStrategyConfigList();
    auto item = std::find(strategyConfigList.begin(), strategyConfigList.end(), strategyName);
    isStandby = item != strategyConfigList.end();
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ReportPowerOverused(const std::string &module, uint32_t level)
{
    STANDBYSERVICE_LOGD("[PowerOverused] StandbyServiceImpl: power overused, "
        "modue name: %{public}s, level: %{public}u", module.c_str(), level);

    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("[PowerOverused] Caller permission denied.");
        return checkRet;
    }

    HandlePowerOverused(0, module, level);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ReportSceneInfo(uint32_t resType, int64_t value, const std::string &sceneInfo)
{
    if (auto checkRet = CheckCallerPermission(ReasonCodeEnum::REASON_NATIVE_API); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }
    StandbyMessage standbyMessage {resType};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("value", static_cast<int32_t>(value));
    standbyMessage.want_->SetParam("sceneInfo", sceneInfo);
    DispatchEvent(standbyMessage);
    return ERR_OK;
}

ErrCode StandbyServiceImpl::ReportDeviceStateChanged(int32_t type, bool enabled)
{
    if (!IsServiceReady()) {
        return ERR_STANDBY_SYS_NOT_READY;
    }

    if (auto checkRet = CheckCallerPermission(); checkRet != ERR_OK) {
        STANDBYSERVICE_LOGE("caller permission denied.");
        return checkRet;
    }

    STANDBYSERVICE_LOGI("device state changed, state type: %{public}d, enabled: %{public}d",
        static_cast<int32_t>(type), enabled);
    DeviceStateCache::GetInstance()->SetDeviceState(type, enabled);
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

void StandbyServiceImpl::HandleChargeStateChanged(const int64_t value)
{
    auto event = value == 0 ? EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING :
        EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING;
    DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT, event));
}

void StandbyServiceImpl::HandleScreenClickRecognize(const int64_t value)
{
    StandbyMessage standbyMessage {StandbyMessageType::SCREEN_CLICK_RECOGNIZE};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("clickType", static_cast<int32_t>(value));
    DispatchEvent(standbyMessage);
}

void StandbyServiceImpl::HandleBTServiceEvent(const int64_t value, const std::string &sceneInfo)
{
    STANDBYSERVICE_LOGI("HandleBTSerciceEvent value: %{public}lld", value);
    nlohmann::json payload = nlohmann::json::parse(sceneInfo, nullptr, false);
    if (payload.is_discarded()) {
        STANDBYSERVICE_LOGE("parse json failed");
    }
    if (value == ResourceSchedule::ResType::BtServiceEvent::GATT_APP_REGISTER) {
        if (!payload.contains("ACTION") || !payload.contains("ADDRESS") || !payload.contains("PID") ||
            !payload.at("ACTION").is_string() || !payload.at("ADDRESS").is_string() || !payload.at("PID").is_string()) {
            STANDBYSERVICE_LOGE("Bt Gatt Register info is valid");
            return;
        }
        std::string action = payload["ACTION"].get<std::string>();
        std::string address = payload["ADDRESS"].get<std::string>();
        int32_t uid = atoi(payload["UID"].get<std::string>().c_str());
        StandbyMessage standbyMessage {StandbyMessageType::GATT_APP_REGISTER};
        standbyMessage.want_ = AAFwk::Want {};
        standbyMessage.want_->SetParam("ACTION", action);
        standbyMessage.want_->SetParam("ADDRESS", address);
        standbyMessage.want_->SetParam("UID", uid);
        DispatchEvent(standbyMessage);
    } else if (value == ResourceSchedule::ResType::BtServiceEvent::GATT_CONNECT_STATE) {
        if (!payload.contains("ADDRESS") || !payload.contains("STATE") ||
            !payload.at("ADDRESS").is_string() || !payload.at("STATE").is_string()) {
            STANDBYSERVICE_LOGE("Bt Gatt Connection info is valid");
            return;
        }
        std::string address = payload["ADDRESS"].get<std::string>();
        int32_t state = atoi(payload["STATE"].get<std::string>().c_str());
        StandbyMessage standbyMessage {StandbyMessageType::GATT_CONNECT_STATE};
        standbyMessage.want_ = AAFwk::Want {};
        standbyMessage.want_->SetParam("ADDRESS", address);
        standbyMessage.want_->SetParam("STATE", state);
        DispatchEvent(standbyMessage);
    }
}

void StandbyServiceImpl::HandleBrokerGattConnect(const int64_t value, const std::string &sceneInfo)
{
    STANDBYSERVICE_LOGI("HandleBrokerGattConnect value: %{public}lld", value);
    nlohmann::json payload = nlohmann::json::parse(sceneInfo, nullptr, false);
    if (payload.is_discarded()) {
        STANDBYSERVICE_LOGE("parse json failed");
    }
    if (value) {
        if (!payload.contains("clientIf") || !payload.contains("pkg") || !payload.at("clientIf").is_string() ||
            !payload.at("pkg").is_string()) {
            STANDBYSERVICE_LOGE("Broker Gatt connect info is valid");
            return;
        }
        std::string pkg = payload["pkg"].get<std::string>();
        int32_t clientIf = atoi(payload["clientIf"].get<std::string>().c_str());
        bool connect = true;
        StandbyMessage standbyMessage {StandbyMessageType::BROKER_GATT_CONNECT};
        standbyMessage.want_ = AAFwk::Want {};
        standbyMessage.want_->SetParam("clientIf", clientIf);
        standbyMessage.want_->SetParam("pkg", pkg);
        standbyMessage.want_->SetParam("connect", connect);
        DispatchEvent(standbyMessage);
    } else {
        if (!payload.contains("clientIf") || !payload.at("clientIf").is_string()) {
            STANDBYSERVICE_LOGE("Broker Gatt disconnect info is valid");
            return;
        }
        int32_t clientIf = atoi(payload["clientIf"].get<std::string>().c_str());
        bool connect = false;
        StandbyMessage standbyMessage {StandbyMessageType::BROKER_GATT_CONNECT};
        standbyMessage.want_ = AAFwk::Want {};
        standbyMessage.want_->SetParam("clientIf", clientIf);
        standbyMessage.want_->SetParam("connect", connect);
        DispatchEvent(standbyMessage);
    }
}

void StandbyServiceImpl::HandleMmiInputPowerKeyDown(const int64_t value)
{
    StandbyMessage standbyMessage {StandbyMessageType::MMI_INPUT_POWER_KEY_DOWN};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("keyCode", value);
    DispatchEvent(standbyMessage);
}

void StandbyServiceImpl::HandleReportFileSizeEvent()
{
    std::vector<std::string> currentDir;
    const std::vector<std::string> directories = {DEVICE_STANDBY_DIR, DEVICE_STANDBY_RDB_DIR};

    for (const auto& dirPath : directories) {
        bool result = ResourceSchedule::ResCommonUtil::DirIterator(dirPath, currentDir);
        if (!result) {
            STANDBYSERVICE_LOGE("Failed to iterate directory: %{public}s", dirPath.c_str());
            return;
        }
    }
    ResourceSchedule::ResschedEventReporter::GetInstance().ReportFileSizeEvent(currentDir);
}

void StandbyServiceImpl::DumpOnPowerOverused(const std::vector<std::string> &argsInStr, std::string &result)
{
    constexpr uint16_t DUMP_THREE_PARAM = 3;
    if (argsInStr.size() != DUMP_THREE_PARAM) {
        STANDBYSERVICE_LOGE("DumpOnPowerOverused param check failed, shoule be [--poweroverused module level].");
        return;
    }

    const std::string &module = argsInStr[DUMP_SECOND_PARAM];
    uint32_t level = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str()));
    HandlePowerOverused(0, module, level);
}

void StandbyServiceImpl::DumpOnActionChanged(const std::vector<std::string> &argsInStr, std::string &result)
{
    constexpr uint16_t DUMP_THREE_PARAM = 3;
    if (argsInStr.size() != DUMP_THREE_PARAM) {
        STANDBYSERVICE_LOGE("DumpOnActionChanged param check failed, shoule be [--actionchanged module action].");
        return;
    }

    const std::string &module = argsInStr[DUMP_SECOND_PARAM];
    uint32_t action = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_THIRD_PARAM].c_str()));
    HandleActionChanged(0, module, action);
}

// handle power overused, resType for extend
void StandbyServiceImpl::HandlePowerOverused([[maybe_unused]]uint32_t resType,
    const std::string &module, uint32_t level)
{
    StandbyStateSubscriber::GetInstance()->NotifyPowerOverusedByCallback(module, level);
}

// handle action changed, resType for extend
void StandbyServiceImpl::HandleActionChanged([[maybe_unused]]uint32_t resType,
    const std::string &module, uint32_t action)
{
    StandbyStateSubscriber::GetInstance()->NotifyLowpowerActionByCallback(module, action);
}

void StandbyServiceImpl::HandleResourcesStateChanged(const int64_t value, const std::string &sceneInfo)
{
        bool isApply = false;
        if (value == ResourceSchedule::ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY ||
            value == ResourceSchedule::ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY) {
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

void StandbyServiceImpl::HandleBootCompleted()
{
    DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
        EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED));
}

void StandbyServiceImpl::HandleThermalScenarioReport(const int64_t value, const std::string &sceneInfo)
{
    StandbyMessage standbyMessage {StandbyMessageType::THERMAL_SCENARIO_REPORT};
    standbyMessage.want_ = AAFwk::Want {};
    standbyMessage.want_->SetParam("scenario", value);
    standbyMessage.want_->SetParam("sceneInfo", sceneInfo);
    DispatchEvent(standbyMessage);
}

ErrCode StandbyServiceImpl::HandleCommonEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo)
{
    STANDBYSERVICE_LOGD("HandleCommonEvent resType = %{public}u, value = %{public}lld, sceneInfo = %{public}s",
                        resType, (long long)(value), sceneInfo.c_str());
    switch (resType) {
        case ResourceSchedule::ResType::RES_TYPE_SCREEN_STATUS:
            HandleScreenStateChanged(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_CHARGING_DISCHARGING:
            HandleChargeStateChanged(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_USB_DEVICE:
            DispatchEvent(StandbyMessage(StandbyMessageType::COMMON_EVENT,
                (value ? EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED :
                    EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED)));
            break;
        case ResourceSchedule::ResType::RES_TYPE_CALL_STATE_CHANGED:
            HandleCallStateChanged(sceneInfo);
            break;
        case ResourceSchedule::ResType::RES_TYPE_WIFI_P2P_STATE_CHANGED:
            HandleP2PStateChanged(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_CLICK_RECOGNIZE:
            HandleScreenClickRecognize(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_MMI_INPUT_POWER_KEY:
            HandleMmiInputPowerKeyDown(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT:
            HandleBTServiceEvent(value, sceneInfo);
            break;
        case ResourceSchedule::ResType::RES_TYPE_REPORT_BOKER_GATT_CONNECT:
            HandleBrokerGattConnect(value, sceneInfo);
            break;
        case ResourceSchedule::ResType::RES_TYPE_POWER_MODE_CHANGED:
            HandlePowerModeChanged(value);
            break;
        case ResourceSchedule::ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED:
            HandleResourcesStateChanged(value, sceneInfo);
            break;
        case ResourceSchedule::ResType::RES_TYPE_BOOT_COMPLETED:
            HandleBootCompleted();
            break;
        case ResourceSchedule::ResType::RES_TYPE_THERMAL_SCENARIO_REPORT:
            HandleThermalScenarioReport(value, sceneInfo);
            break;
        default:
            AppEventHandler(resType, value, sceneInfo);
            break;
    }
    return ERR_OK;
}

void StandbyServiceImpl::HandlePowerModeChanged(const int64_t value)
{
    StandbyMessage message(StandbyMessageType::COMMON_EVENT);
    message.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
    message.want_ = AAFwk::Want {};
    message.want_->SetParam("current_power_mode", value);
    DispatchEvent(message);
}

void StandbyServiceImpl::AppEventHandler(const uint32_t resType, const int64_t value, const std::string &sceneInfo)
{
    if (resType == ResourceSchedule::ResType::RES_TYPE_APP_INSTALL_UNINSTALL &&
        (value == ResourceSchedule::ResType::AppInstallStatus::APP_UNINSTALL ||
         value == ResourceSchedule::ResType::AppInstallStatus::APP_CHANGED ||
         value == ResourceSchedule::ResType::AppInstallStatus::APP_REPLACED ||
         value == ResourceSchedule::ResType::AppInstallStatus::BUNDLE_REMOVED ||
         value == ResourceSchedule::ResType::AppInstallStatus::APP_FULLY_REMOVED)
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
    } else if (resType == ResourceSchedule::ResType::RES_TYPE_TIMEZONE_CHANGED ||
               resType == ResourceSchedule::ResType::RES_TYPE_NITZ_TIMEZONE_CHANGED ||
               resType == ResourceSchedule::ResType::RES_TYPE_TIME_CHANGED ||
               resType == ResourceSchedule::ResType::RES_TYPE_NITZ_TIME_CHANGED) {
        handler_->PostTask([]() {StandbyServiceImpl::GetInstance()->ResetTimeObserver(); });
    }
}

void StandbyServiceImpl::DispatchEvent(const StandbyMessage& message)
{
    if (!IsServiceReady()) {
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
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_ON_POWER_OVERUSED) {
        DumpOnPowerOverused(argsInStr, result);
    } else if (argsInStr[DUMP_FIRST_PARAM] == DUMP_ON_ACTION_CHANGED) {
        DumpOnActionChanged(argsInStr, result);
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
        std::shared_ptr<ResourceRequest> resourceRequest = std::make_shared<ResourceRequest>(allowType,
            uid, name, duration, "dump", std::atoi(argsInStr[DUMP_SEVENTH_PARAM].c_str()));
        ApplyAllowResource(*resourceRequest);
        result += "add one object to allow list\n";
    } else if (argsInStr[DUMP_SECOND_PARAM] == "--unapply") {
        uint32_t allowType = static_cast<uint32_t>(std::atoi(argsInStr[DUMP_FIFTH_PARAM].c_str()));
        std::shared_ptr<ResourceRequest> resourceRequest = std::make_shared<ResourceRequest>(allowType,
            uid, name, 0, "dump", std::atoi(argsInStr[DUMP_SEVENTH_PARAM].c_str()));
        UnapplyAllowResource(*resourceRequest);
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

DeviceStateCache::DeviceStateCache()
{
    deviceState_ = {false, false, false};
}

DeviceStateCache::~DeviceStateCache() {}

std::shared_ptr<DeviceStateCache> DeviceStateCache::GetInstance()
{
    return DelayedSingleton<DeviceStateCache>::GetInstance();
}

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
