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
#ifndef DEVICE_STANDBY_EXT_BASE_NETWPRK_STRATEGY_H
#define DEVICE_STANDBY_EXT_BASE_NETWPRK_STRATEGY_H

#include "ibase_strategy.h"

namespace OHOS {
namespace DevStandbyMgr {
struct NetLimtedAppInfo {
    std::string name_ {""};
    uint8_t appExemptionFlag_ {0};
};

class BaseNetworkStrategy : public IBaseStrategy {
public:
    /**
     * @brief BaseNetworkStrategy HandleEvent by StandbyMessage.
     */
    void HandleEvent(const StandbyMessage& message) override;

    /**
     * @brief BaseNetworkStrategy OnCreated.
     *
     * @return ERR_OK if OnCreated success, failed with other code.
     */
    ErrCode OnCreated() override;

    /**
     * @brief BaseNetworkStrategy OnDestroy.
     *
     * @return ERR_OK if OnDestroy success, failed with other code.
     */
    ErrCode OnDestroy() override;
    void ShellDump(const std::vector<std::string>& argsInStr, std::string& result) override;

    /**
     * @brief set net limit status.
     *
     * @return NetPolicy handle ret.
     */
    virtual int32_t HandleDeviceIdlePolicy(bool enableFirewall);
protected:
    /**
     * @brief stop power save mode, clean all exemption uid.
     */
    void ResetFirewallAllowList();

    /**
     * @brief get all apps, system apps defaultly not be restricted.
     */
    virtual void SetFirewallAllowedList(const std::vector<uint32_t>& uids, bool isAdded) = 0;

    /**
     * @brief set net limited mode, if netLimited is true, start net limited mode, else stop net limited mode.
     */
    ErrCode SetFirewallStatus(bool enableFirewall);

    /**
     * @brief update exemption list when received exemption list changed event.
     */
    virtual ErrCode UpdateExemptionList(const StandbyMessage& message);

    /**
     * @brief update resource config when received message of day night switch or sleep stat change.
     */
    virtual ErrCode UpdateFirewallAllowList();

    /**
     * @brief start net limited mode when has recerved reletive event.
     */
    virtual ErrCode EnableNetworkFirewall(const StandbyMessage& message);

    /**
     * @brief stop net limited mode when has recerved reletive event.
     */
    virtual ErrCode DisableNetworkFirewall(const StandbyMessage& message);

    /**
     * @brief update status of app with background task.
     */
    virtual ErrCode UpdateBgTaskAppStatus(const StandbyMessage& message);

    /**
     * @brief handle process creted or died.
     */
    virtual void HandleProcessStatusChanged(const StandbyMessage& message);
    /**
     * @brief application in exemption list or with bgtask will not be proxied.
     */
    virtual ErrCode InitNetLimitedAppInfo();

    /**
     * @brief update allow uid and send to  Firewall.
     */
    virtual void SetNetAllowApps(bool isAllow);

protected:
    void ResetFirewallStatus(const StandbyMessage& message);

    ErrCode EnableNetworkFirewallInner();
    ErrCode DisableNetworkFirewallInner();
    ErrCode GetAllRunningAppInfo();

    ErrCode GetForegroundApplications();
    // get backgroundtask, including continuous task and transient, defaultly not be constricted.
    ErrCode GetBackgroundTaskApp();
    // get running work scheduler task and add work_scheduler flag to relative apps.
    ErrCode GetWorkSchedulerTask();
    void AddExemptionFlagByUid(int32_t uid, uint8_t flag);
    // get exemption app and restrict list from standby service
    ErrCode GetExemptionConfig();
    ErrCode GetExemptionConfigForApp(NetLimtedAppInfo& appInfo, const std::string& bundleName);

    void AddExemptionFlag(uint32_t uid, const std::string& bundleName, uint8_t flag);
    void RemoveExemptionFlag(uint32_t uid, uint8_t flag);
    void GetAndCreateAppInfo(uint32_t uid, const std::string& bundleName);
    bool GetExemptedFlag(uint8_t appNoExemptionFlag, uint8_t appExemptionFlag);
protected:
    static bool isFirewallEnabled_;
    bool isIdleMaintence_ {false};
    static std::unordered_map<std::int32_t, NetLimtedAppInfo> netLimitedAppInfo_;
    uint32_t nightExemptionTaskType_ {0};
    uint32_t condition_ {0};
    const static std::int32_t NETMANAGER_SUCCESS = 0;
    const static std::int32_t NETMANAGER_ERR_STATUS_EXIST = 2100209;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif // DEVICE_STANDBY_EXT_BASE_NETWPRK_STRATEGY_H
