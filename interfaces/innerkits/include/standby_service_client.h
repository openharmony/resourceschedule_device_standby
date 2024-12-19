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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INNERKITS_INCLUDE_STANDBY_SERVICE_CLIENT_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INNERKITS_INCLUDE_STANDBY_SERVICE_CLIENT_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "istandby_service.h"
#include "allow_info.h"
#include "resource_request.h"
#include "standby_service_errors.h"
#include "istandby_service_subscriber.h"
#include "res_data.h"

namespace OHOS {
namespace DevStandbyMgr {

enum class DeviceStateType: int32_t {
    DIS_COMP_CHANGE = 0,
    TELEPHONE_STATE_CHANGE,
    WIFI_P2P_CHANGE,
};

enum class PowerOverusedLevel : uint32_t {
    NORMAL = 0,
    MINOR,
    WARNING,
    SERIOUS,
    EXTREME,
    FATAL,
};

class StandbyServiceClient {
public:
    StandbyServiceClient();

    virtual ~StandbyServiceClient();

    static StandbyServiceClient& GetInstance();

    /**
     * @brief Subscribes sleep state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber);

    /**
     * @brief Unsubscribes sleep state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber);

    /**
     * @brief add allow list for some services or apps.
     *
     * @param resourceRequest resource to be added.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest);

    /**
     * @brief remove uid with allow type from allow list.
     *
     * @param resourceRequest resource to be removed.
     * @return ErrCode ErrCode ERR_OK if success, others if fail.
     */
    ErrCode UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest);

    /**
     * @brief Get the Allow List object.
     *
     * @param allowType the allow type to be retrieved.
     * @param allowInfoList result represents allowed types and apps.
     * @param reasonCode represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, else fail.
     */
    ErrCode GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode);

    /**
     * @brief Get the Restrict List object.
     *
     * @param restrictType the restrict type to be retrieved.
     * @param restrictInfoList result represents restricted types and apps.
     * @param reasonCode represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode);

    /**
     * @brief Construct a new Report Work Scheduler Status object.
     *
     * @param started true if the work is triggered, else false.
     * @param uid uid of the applicatoin.
     * @param bundleName bundleName of the application.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName);

    /**
     * @brief Whether the restriction strategy enbaled or not.
     *
     * @param strategyName the strategy name.
     * @param enabled true if the strategy is enabled.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode IsStrategyEnabled(const std::string& strategyName, bool& isEnabled);

    /**
     * @brief Report event when device state change, such as discomponent device, bluetooth socket..
     *
     * @param type type of device state.
     * @param enabled true if the device state is on.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode ReportDeviceStateChanged(DeviceStateType type, bool enabled);

    /**
     * @brief query if the device is in standby mode;
     *
     * @param isStandby true if device in standby, else false.
     * @return ErrCode ERR_OK if success, else fail.
     */
    ErrCode IsDeviceInStandby(bool& isStandby);

    /**
     * @brief set nat timeout interval;
     *
     * @param type detect type.
     * @param enable adjust or not.
     * @param interval nat timeout interval.
     * @return ErrCode ERR_OK if success, else fail.
     */
    ErrCode SetNatInterval(uint32_t& type, bool& enable, uint32_t& interval);

    /**
     * @brief Unified handling of events;
     *
     * @param resData event data.
     * @return ErrCode ERR_OK if success, else fail.
     */
    ErrCode HandleEvent(const std::shared_ptr<ResourceSchedule::ResData> &resData);

    /**
     * @brief Report event when a module power energy is overused according to the information from XPower
     *
     * @param module module name
     * @param level rate of power overused
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode ReportPowerOverused(const std::string &module, uint32_t level);

private:
    bool GetStandbyServiceProxy();
    void ResetStandbyServiceClient();

    class StandbyServiceDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit StandbyServiceDeathRecipient(StandbyServiceClient& standbyServiceClient);

        ~StandbyServiceDeathRecipient() override;

        void OnRemoteDied(const wptr<IRemoteObject>& object) override;

    private:
        StandbyServiceClient& standbyServiceClient_;
    };

private:
    std::mutex mutex_;
    sptr<IStandbyService> standbyServiceProxy_;
    sptr<StandbyServiceDeathRecipient> deathRecipient_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INNERKITS_INCLUDE_STANDBY_SERVICE_CLIENT_H
