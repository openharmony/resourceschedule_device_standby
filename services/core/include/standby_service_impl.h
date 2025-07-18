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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_IMPL_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_IMPL_H
#ifdef STANDBY_SERVICE_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include <array>
#include <map>
#include <memory>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "accesstoken_kit.h"
#include "allow_info.h"
#include "allow_record.h"
#include "app_mgr_client.h"
#include "app_mgr_helper.h"
#include "app_state_observer.h"
#include "common_event_observer.h"
#include "event_runner.h"
#include "event_handler.h"
#include "iconstraint_manager_adapter.h"
#include "ilistener_manager_adapter.h"
#include "ipc_skeleton.h"
#include "istandby_service_subscriber.h"
#include "istate_manager_adapter.h"
#include "istrategy_manager_adapter.h"
#include "nlohmann/json.hpp"
#include "resource_request.h"
#include "res_type.h"
#include "singleton.h"
#include "standby_state_subscriber.h"

namespace OHOS {
namespace DevStandbyMgr {
enum class TelCallState {
    /**
    * Indicates the call is unknown
    */
    CALL_STATUS_UNKNOWN = -1,
    /**
    * Indicates the call is disconnected
    */
    CALL_STATUS_DISCONNECTED = 6,
    /**
    * Indicates the call is idle
    */
    CALL_STATUS_IDLE = 8,
};
enum P2pState {
    P2P_STATE_NONE = 0,
    P2P_STATE_IDLE,
    P2P_STATE_STARTING,
    P2P_STATE_STARTED,
    P2P_STATE_CLOSING,
    P2P_STATE_CLOSED,
};

struct CloneFileHead {
    char moduleName[32];
    uint32_t fileOffset;
    uint32_t fileSize;
    uint8_t data[0];
};

class StandbyServiceImpl : public std::enable_shared_from_this<StandbyServiceImpl> {
DECLARE_DELAYED_SINGLETON(StandbyServiceImpl);
public:
    static std::shared_ptr<StandbyServiceImpl> GetInstance();
    bool Init();
    void InitReadyState();
    ErrCode RegisterCommEventObserver();
    ErrCode RegisterAppStateObserver();
    ErrCode RegisterTimeObserver();
    ErrCode UnregisterCommEventObserver();
    ErrCode UnregisterAppStateObserver();
    ErrCode UnregisterTimeObserver();
    ErrCode ResetTimeObserver();
    void DayNightSwitchCallback();
    ErrCode RegisterPlugin(const std::string& pluginName);
    void UninitReadyState();
    void UnInit();
    ErrCode SubscribeBackupRestoreCallback(const std::string& moduleName,
        const std::function<ErrCode(std::vector<char>&)>& onBackupFunc,
        const std::function<ErrCode(std::vector<char>&)>& onRestoreFunc);
    ErrCode UnsubscribeBackupRestoreCallback(const std::string& moduleName);
    ErrCode OnBackup(MessageParcel& data, MessageParcel& reply);
    ErrCode OnRestore(MessageParcel& data, MessageParcel& reply);

    ErrCode SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber);
    ErrCode UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber);
    ErrCode ApplyAllowResource(ResourceRequest& resourceRequest);
    ErrCode UnapplyAllowResource(ResourceRequest& resourceRequest);
    ErrCode GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode);
    ErrCode GetEligiableRestrictSet(uint32_t allowType, const std::string& strategyName,
        uint32_t resonCode, std::set<std::string>& restrictSet);
    ErrCode IsDeviceInStandby(bool& isStandby);
    ErrCode ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName);
    ErrCode GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode);
    ErrCode IsStrategyEnabled(const std::string& strategyName, bool& isEnabled);
    ErrCode ReportDeviceStateChanged(int32_t type, bool enabled);
    ErrCode HandleCommonEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo);
    ErrCode ReportPowerOverused(const std::string &module, uint32_t level);
    ErrCode ReportSceneInfo(uint32_t resType, int64_t value, const std::string &sceneInfo);

    void RegisterPluginInner(IConstraintManagerAdapter* constraintManager,
        IListenerManagerAdapter* listenerManager,
        IStrategyManagerAdapter* strategyManager,
        IStateManagerAdapter* stateManager);

    std::shared_ptr<AppExecFwk::EventHandler>& GetHandler();
    std::shared_ptr<IConstraintManagerAdapter>& GetConstraintManager();
    std::shared_ptr<IListenerManagerAdapter>& GetListenerManager();
    std::shared_ptr<IStrategyManagerAdapter>& GetStrategyManager();
    std::shared_ptr<IStateManagerAdapter>& GetStateManager();

    ErrCode RemoveAppAllowRecord(int32_t uid, const std::string &bundleName, bool resetAll);

    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result);
    void ShellDumpInner(const std::vector<std::string>& argsInStr, std::string& result);
    void GetAllowListInner(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode);
    void DispatchEvent(const StandbyMessage& message);
    bool IsDebugMode();
    bool IsServiceReady();
    void UpdateSaDependValue(const bool& isAdd, const uint32_t& saId);
    uint32_t GetSaDependValue();

    void OnProcessStatusChanged(int32_t uid, int32_t pid, const std::string& bundleName, bool isCreated);
private:
    StandbyServiceImpl(const StandbyServiceImpl&) = delete;
    StandbyServiceImpl& operator= (const StandbyServiceImpl&) = delete;
    StandbyServiceImpl(StandbyServiceImpl&&) = delete;
    StandbyServiceImpl& operator= (StandbyServiceImpl&&) = delete;
    void ApplyAllowResInner(const ResourceRequest& resourceRequest, int32_t pid);
    void UpdateRecord(std::shared_ptr<AllowRecord>& allowRecord, const ResourceRequest& resourceRequest);
    void UnapplyAllowResInner(int32_t uid, const std::string& name, uint32_t allowType,  bool removeAll);
    void GetTemporaryAllowList(uint32_t allowTypeIndex, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode);
    void GetPersistAllowList(uint32_t allowTypeIndex, std::vector<AllowInfo>& allowInfoList, bool isAllow, bool isApp);
    void GetRestrictListInner(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode);
    void NotifyAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType, bool added);
    std::string BuildBackupReplyCode(int32_t replyCode);

    void RecoverTimeLimitedTask();
    bool ParsePersistentData();
    void GetPidAndProcName(std::unordered_map<int32_t, std::string>& pidNameMap);
    void DumpPersistantData();
    uint32_t dependsReady_ = 0;

    ErrCode CheckCallerPermission(uint32_t reasonCode = ReasonCodeEnum::REASON_APP_API);
    ErrCode IsSystemAppWithPermission(int32_t uid, Security::AccessToken::AccessTokenID tokenId, uint32_t reasonCode);
    ErrCode CheckNativePermission(Security::AccessToken::AccessTokenID tokenId);
    bool CheckAllowTypeInfo(uint32_t allowType);
    uint32_t GetExemptedResourceType(uint32_t resourceType);
    std::vector<int32_t> QueryRunningResourcesApply(const int32_t uid, const std::string& bundleName);
    int32_t GetUserIdByUid(int32_t uid);

    void DumpUsage(std::string& result);
    void DumpShowDetailInfo(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpAllowListInfo(std::string& result);
    void DumpStandbyConfigInfo(std::string& result);
    void DumpChangeModeSwitch(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpEnterSpecifiedState(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpModifyAllowList(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpSubScriberObserver(const std::vector<std::string>& argsInStr, std::string& result);

    void DumpTurnOnOffSwitch(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpChangeConfigParam(const std::vector<std::string>& argsInStr, std::string& result);
    void DumpPushStrategyChange(const std::vector<std::string>& argsInStr, std::string& result);
    // dispatch dumper command to plugin
    void OnPluginShellDump(const std::vector<std::string>& argsInStr, std::string& result);
    void AppEventHandler(const uint32_t resType, const int64_t value, const std::string &sceneInfo);
    void HandleCallStateChanged(const std::string &sceneInfo);
    void HandleP2PStateChanged(int32_t state);
    void HandleReportFileSizeEvent();
    void HandleScreenClickRecognize(const int64_t value);
    void HandleMmiInputPowerKeyDown(const int64_t value);
    void HandleBTServiceEvent(const int64_t value, const std::string &sceneInfo);
    void HandleBrokerGattConnect(const int64_t value, const std::string &sceneInfo);
    void HandleThermalScenarioReport(const int64_t value, const std::string &sceneInfo);
    void HandlePowerModeChanged(const int64_t value);
    void HandleResourcesStateChanged(const int64_t value, const std::string &sceneInfo);
    void HandleScreenStateChanged(const int64_t value);
    void HandleChargeStateChanged(const int64_t value);
    void HandleBootCompleted();
    
    // handle abnormal power use
    void HandlePowerOverused([[maybe_unused]]uint32_t resType, const std::string &module, uint32_t level);
    void DumpOnPowerOverused(const std::vector<std::string> &argsInStr, std::string &result);

    void HandleActionChanged([[maybe_unused]]uint32_t resType, const std::string &module, uint32_t action);
    void DumpOnActionChanged(const std::vector<std::string> &argsInStr, std::string &result);

private:
    std::atomic<bool> isServiceReady_ {false};
    sptr<AppStateObserver> appStateObserver_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
    std::mutex appStateObserverMutex_ {};
    std::mutex eventObserverMutex_ {};
    std::recursive_mutex timerObserverMutex_ {};
    std::mutex allowRecordMutex_ {};
    std::mutex backupRestoreMutex_ {};
    std::map<std::string, std::function<ErrCode(std::vector<char>&)>> onBackupFuncMap_ {};
    std::map<std::string, std::function<ErrCode(std::vector<char>&)>> onRestoreFuncMap_ {};
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_ {nullptr};
    std::shared_ptr<CommonEventObserver> commonEventObserver_ {nullptr};
    uint64_t dayNightSwitchTimerId_ {0};
    std::unordered_map<std::string, std::shared_ptr<AllowRecord>> allowInfoMap_ {};
    bool ready_ = false;
    void* registerPlugin_ {nullptr};
    std::shared_ptr<IConstraintManagerAdapter> constraintManager_ {nullptr};
    std::shared_ptr<IListenerManagerAdapter> listenerManager_ {nullptr};
    std::shared_ptr<IStrategyManagerAdapter> strategyManager_ {nullptr};
    std::shared_ptr<IStateManagerAdapter> standbyStateManager_ {nullptr};
    bool debugMode_ {false};
};

class DeviceStateCache {
DECLARE_DELAYED_SINGLETON(DeviceStateCache);
public:
    static std::shared_ptr<DeviceStateCache> GetInstance();
    bool SetDeviceState(int32_t type, bool enabled);
    bool GetDeviceState(int32_t type);
private:
    DeviceStateCache(const DeviceStateCache&) = delete;
    DeviceStateCache& operator= (const DeviceStateCache&) = delete;
    DeviceStateCache(DeviceStateCache&&) = delete;
    DeviceStateCache& operator= (DeviceStateCache&&) = delete;
    std::mutex mutex_ {};
    const static std::int32_t DEVICE_STATE_NUM = 3;
    std::array<bool, DEVICE_STATE_NUM> deviceState_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_SERVICES_CORE_INCLUDE_STANDBY_SERVICE_IMPL_H
