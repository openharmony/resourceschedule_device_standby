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
#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_RUNNINGLOCK_STRATEGY_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_RUNNINGLOCK_STRATEGY_H
#include "ibase_strategy.h"

#include <unordered_map>
#include <set>
#include <string>

namespace OHOS {
namespace DevStandbyMgr {
struct ProxiedAppInfo {
    std::string name_ {""};
    int32_t uid_ {-1};
    int32_t pid_ {-1};
    uint8_t appExemptionFlag_ {0};
};

struct ProxiedProcInfo {
    std::string name_ {""};
    int32_t uid_ {-1};
    std::set<int32_t> pids_ {};
    uint8_t appExemptionFlag_ {0};
};

class RunningLockStrategy : public IBaseStrategy {
public:
    /**
     * @brief RunningLockStrategy HandleEvent by StandbyMessage.
     */
    void HandleEvent(const StandbyMessage& message) override;

    /**
     * @brief RunningLockStrategy OnCreated.
     *
     * @return ERR_OK if OnCreated success, failed with other code.
     */
    ErrCode OnCreated() override;

    /**
     * @brief RunningLockStrategy OnDestroy.
     *
     * @return ERR_OK if OnDestroy success, failed with other code.
     */
    ErrCode OnDestroy() override;
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) override;
protected:
    // native processes will not be proxied, except in restriction list
    virtual ErrCode InitNativeProcInfo();
    // add exemption flag according to message
    virtual void AddExemptionFlag(int32_t uid, const std::string& name, uint8_t flag);
    // remove exemption flag according to message
    virtual void RemoveExemptionFlag(int32_t uid, const std::string& name, uint8_t flag);
    // proxy app and native process
    virtual ErrCode ProxyAppAndProcess(bool isProxied);
    // clear record of proxied app and process
    virtual void ClearProxyRecord();

    void SetProxiedAppList(std::vector<std::pair<int32_t, int32_t>>& proxiedAppList,
        const ProxiedProcInfo& info);
    void ProxyRunningLockList(bool isProxied, const std::vector<std::pair<int32_t, int32_t>>& proxiedAppList);
private:
    // update exemtion list when received exemtion list changed event
    ErrCode UpdateExemptionList(const StandbyMessage& message);
    // update resource config when received condition changed event
    ErrCode UpdateResourceConfig(const StandbyMessage& message);
    ErrCode StartProxy(const StandbyMessage& message);
    ErrCode StartProxyInner();
    ErrCode StopProxy(const StandbyMessage& message);
    ErrCode StopProxyInner();
    // add exemption to background task and work_scheduler task, unproxy running lock
    ErrCode UpdateBgTaskAppStatus(const StandbyMessage& message);
    void ResetProxyStatus(const StandbyMessage& message);

    // application in exemption list or with bgtask will not be proxied.
    ErrCode InitProxiedAppInfo();
    // get all apps, system apps defaultly not be restricted.
    ErrCode GetAllAppInfos();
    ErrCode GetAllRunningAppInfo();
    ErrCode GetForegroundApplications();
    // get running work scheduler task and add work_scheduler flag to relative apps.
    ErrCode GetWorkSchedulerTask();
    // get background task, including continuous task and transient, defaultly not be constricted.
    ErrCode GetBackgroundTaskApp();
    // get exemption app list from standby service
    ErrCode GetExemptionConfig();

    // when process is created or died, update proxy app info
    void HandleProcessStatusChanged(const StandbyMessage& message);

    void GetAndCreateAppInfo(uint32_t uid, uint32_t pid, const std::string& bundleName);
    ErrCode GetExemptionConfigForApp(ProxiedProcInfo& appInfo, const std::string& bundleName);

    void DumpShowDetailInfo(const std::vector<std::string>& argsInStr, std::string& result);
protected:
    bool isProxied_ {false};
private:
    bool isIdleMaintence_ {false};
    // proxyed app and native process info
    std::unordered_map<std::string, ProxiedProcInfo> proxiedAppInfo_;

    std::unordered_map<std::int32_t, std::string> uidBundleNmeMap_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_STRATEGY_INCLUDE_RUNNINGLOCK_STRATEGY_H