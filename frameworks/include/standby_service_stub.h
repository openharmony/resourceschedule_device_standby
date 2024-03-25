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

#ifndef FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_STUB_H
#define FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_STUB_H

#include <map>

#include <iremote_stub.h>
#include <nocopyable.h>

#include "istandby_service.h"

namespace OHOS {
namespace DevStandbyMgr {
class StandbyServiceStub : public IRemoteStub<IStandbyService> {
public:
    StandbyServiceStub(bool serialInvokeFlag = true) : IRemoteStub(serialInvokeFlag) {};
    ~StandbyServiceStub() override = default;
    DISALLOW_COPY_AND_MOVE(StandbyServiceStub);

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

private:
    static const std::map<uint32_t,
        std::function<ErrCode(StandbyServiceStub*, MessageParcel&, MessageParcel&)>> interfaces_;
    ErrCode HandleSubscribeStandbyCallback(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleUnsubscribeStandbyCallback(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleApplyAllowResource(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleUnapplyAllowResource(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleGetAllowList(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleIsDeviceInStandby(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleReportWorkSchedulerStatus(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleGetRestrictList(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleIsStrategyEnabled(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleReportDeviceStateChanged(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleCommonEvent(MessageParcel& data, MessageParcel& reply);
};
}  // namespace DevStandbyMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_STANDBY_SERVICE_FRAMEWORKS_INCLUDE_STANDBY_SERVICE_STUB_H