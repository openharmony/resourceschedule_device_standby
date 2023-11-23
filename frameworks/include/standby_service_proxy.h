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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_PROXY_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "istandby_service.h"

namespace OHOS {
namespace DevStandbyMgr {
class StandbyServiceProxy final : public IRemoteProxy<IStandbyService> {
public:
    explicit StandbyServiceProxy(const sptr<IRemoteObject>& impl);
    ~StandbyServiceProxy() override;
    DISALLOW_COPY_AND_MOVE(StandbyServiceProxy);

    /**
     * @brief Subscribes standby state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) override;

    /**
     * @brief Unsubscribes standby state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) override;

    /**
     * @brief add allow list for some services or apps.
     *
     * @param resourceRequest resource to be added.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest) override;

    /**
     * @brief remove uid with allow type from allow list.
     *
     * @param resourceRequest resource to be removed.
     * @return ErrCode ErrCode ERR_OK if success, others if fail.
     */
    ErrCode UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest) override;

    /**
     * @brief Get the Allow List object.
     *
     * @param allowType the allow type to be retrieved.
     * @param allowInfoList result represents allowed types and apps.
     * @param isApp represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode) override;

    /**
     * @brief query if the device is in standby mode;
     *
     * @param isStandby true if device in standby, else false.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode IsDeviceInStandby(bool& isStandby) override;

    /**
     * @brief Construct a new Report Work Scheduler Status object.
     *
     * @param started true if the work is triggered, else false.
     * @param uid uid of the applicatoin.
     * @param bundleName bundleName of the application.
     */
    ErrCode ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName) override;

    /**
     * @brief Get the Restrict List object.
     *
     * @param allowType the allow type to be retrieved.
     * @param allowInfoList result represents allowed types and apps.
     * @param reasonCode represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode) override;

    /**
     * @brief Whether the restriction strategy enbaled or not.
     *
     * @param strategyName the strategy name.
     * @param enabled true if the strategy is enabled.
     */
    ErrCode IsStrategyEnabled(const std::string& strategyName, bool& enabled) override;

    /**
     * @brief Report event when device state change.
     *
     * @param type type of device state.
     * @param enabled true if the device state is on.
     */
    ErrCode ReportDeviceStateChanged(DeviceStateType type, bool enabled) override;

    /**
     * @brief Unified handing of events
     *
     * @param resType scene type
     * @param vaule extra scene message
     * @param sceneInfo detail scene message, such as pid, uid and so on
     * @return ErrCode ERR_OK if success, others if fail.
     */
    ErrCode HandleEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo) override;
private:
    ErrCode InnerTransact(uint32_t code, MessageOption& flags, MessageParcel& data, MessageParcel& reply);

    static inline BrokerDelegator<StandbyServiceProxy> delegator_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_PROXY_H
