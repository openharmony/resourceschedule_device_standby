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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_H

#include <string>
#include <vector>

#include <ipc_types.h>
#include <iremote_broker.h>
#include <nocopyable.h>

#include "allow_info.h"
#include "resourcce_request.h"
#include "standby_service_errors.h"
#include "istandby_service_subscriber.h"
#include "standby_res_data.h"

namespace OHOS {
namespace DevStandbyMgr {
enum class DeviceStateType: int32_t {
    DIS_COMP_CHANGE = 0,
    TELEPHONE_STATE_CHANGE,
    WIFI_P2P_CHANGE,
};

class IStandbyService : public IRemoteBroker {
public:
    IStandbyService() = default;
    ~IStandbyService() override = default;
    DISALLOW_COPY_AND_MOVE(IStandbyService);

    /**
     * @brief Subscribes standby state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, others if fail.
     */
    virtual ErrCode SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) = 0;

    /**
     * @brief Unsubscribes standby state change event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, others if fail.
     */
    virtual ErrCode UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber) = 0;

    /**
     * @brief add allow list for some services or apps.
     *
     * @param resourceRequest resource to be added.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest) = 0;

    /**
     * @brief remove uid with allow type from allow list.
     *
     * @param resourceRequest resource to be removed.
     * @return ErrCode ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest) = 0;

    /**
     * @brief Get the Allow List object.
     *
     * @param allowType the allow type to be retrieved.
     * @param allowInfoList result represents allowed types and apps.
     * @param reasonCode represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
        uint32_t reasonCode) = 0;

    /**
     * @brief Get the Restrict List object.
     *
     * @param restrictType the restrict type to be retrieved.
     * @param restrictInfoList result represents restricted types and apps.
     * @param reasonCode represents the reason why invoke the api.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
        uint32_t reasonCode) = 0;

    /**
     * @brief Construct a new Report Work Scheduler Status object.
     *
     * @param started true if the work is triggered, else false.
     * @param uid uid of the applicatoin.
     * @param bundleName bundleName of the application.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName) = 0;

    /**
     * @brief Whether the restriction strategy enbaled or not.
     *
     * @param strategyName the strategy name.
     * @param enabled true if the strategy is enabled.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode IsStrategyEnabled(const std::string& strategyName, bool& isEnabled) = 0;

    /**
     * @brief Report event when device state change, such as discomponent device, bluetooth socket.
     *
     * @param type type of device state.
     * @param enabled true if the device state is on.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode ReportDeviceStateChanged(DeviceStateType type, bool enabled) = 0;

    /**
     * @brief query if the device is in standby mode.
     *
     * @param isStandby true if device in standby, else false.
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode IsDeviceInStandby(bool& isStandby) = 0;

    /**
     * @brief Unified handing of events
     *
     * @param resType scene type
     * @param vaule extra scene message
     * @param sceneInfo detail scene message, such as pid, uid and so on
     * @return ErrCode ERR_OK if success, others if fail.
     */
    virtual ErrCode HandleEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.resourceschedule.IStandbyService");

protected:
    enum InterfaceId : uint32_t {
        SUBSCRIBE_STANDBY_CALLBACK = MIN_TRANSACTION_ID,
        UNSUBSCRIBE_STANDBY_CALLBACK,
        APPLY_ALLOW_RESOURCE,
        UNAPPLY_ALLOW_RESOURCE,
        GET_ALLOW_LIST,
        IS_DEVICE_IN_STANDBY,
        REPORT_WORK_SCHEDULER_STATUS,
        REPORT_DEVICE_STATE_CHANGED,
        HANDLE_EVENT
    };
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_H
