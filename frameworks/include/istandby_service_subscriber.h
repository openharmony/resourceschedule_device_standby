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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_SUBSCRIBER_H

#include <iremote_broker.h>
#include <nocopyable.h>

#include "refbase.h"

namespace OHOS {
namespace DevStandbyMgr {
class IStandbyServiceSubscriber : public IRemoteBroker {
public:
    IStandbyServiceSubscriber() = default;
    ~IStandbyServiceSubscriber() override = default;
    DISALLOW_COPY_AND_MOVE(IStandbyServiceSubscriber);

    /**
     * @brief Called back when the standby state changed.
     *
     * @param napped The device is in the nap mode.
     * @param sleeping The device is in the sleep mode.
     */
    virtual void OnDeviceIdleMode(bool napped, bool sleeping) = 0;

    /**
     * @brief report change of allow list to subscriber.
     *
     * @param uid uid which changed happens in.
     * @param name process name of uid.
     * @param allowType The change of the chang.
     * @param added add or removed.
     */
    virtual void OnAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType, bool added) = 0;

    /**
     * @brief get subscriberName.
     *
     * @return return subscriberName.
     */
    std::string GetSubscriberName()
    {
        return subscriberName_;
    }

    /**
     * @brief set subscriberName.
     *
     * @param name strategy name which will register device_standby callback.
     */
    void SetSubscriberName(std::string name)
    {
        subscriberName_ = name;
    }

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.resourceschedule.IStandbyServiceSubscriber");

private:
    std::string subscriberName_;
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_ISTANDBY_SERVICE_SUBSCRIBER_H