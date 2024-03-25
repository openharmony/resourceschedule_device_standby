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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INCLUDE_STANDBY_SERVICE_SUBSCRIBER_STUB_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INCLUDE_STANDBY_SERVICE_SUBSCRIBER_STUB_H

#include <iremote_stub.h>

#include "istandby_service_subscriber.h"
#include "standby_service_errors.h"

namespace OHOS {
namespace DevStandbyMgr {
class StandbyServiceSubscriberStub : public IRemoteStub<IStandbyServiceSubscriber> {
public:
    StandbyServiceSubscriberStub(bool serialInvokeFlag = true) : IRemoteStub(serialInvokeFlag) {};
    ~StandbyServiceSubscriberStub() override;
    DISALLOW_COPY_AND_MOVE(StandbyServiceSubscriberStub);

    /**
     * @brief Request service code and service data.
     *
     * @param code Service request code.
     * @param data MessageParcel object.
     * @param reply Local service response.
     * @param option Point out async or sync.
     * @return ERR_OK if success, else fail.
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnDeviceIdleMode(bool napped, bool sleeping) override;
    void OnAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType, bool added) override;
private:
    ErrCode HandleOnDeviceIdleMode(MessageParcel& data);
    ErrCode HandleOnAllowListChanged(MessageParcel& data);

    ErrCode OnRemoteRequestInner(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_INTERFACE_INCLUDE_STANDBY_SERVICE_SUBSCRIBER_STUB_H