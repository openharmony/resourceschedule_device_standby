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

#include "standby_service_subscriber_stub.h"

#include <errors.h>
#include <ipc_skeleton.h>

#include "standby_ipc_interface_code.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
StandbyServiceSubscriberStub::~StandbyServiceSubscriberStub() {}

ErrCode StandbyServiceSubscriberStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = StandbyServiceSubscriberStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        STANDBYSERVICE_LOGW("StandbyServiceSubscriberStub Local descriptor not match remote.");
        return ERR_TRANSACTION_FAILED;
    }

    return OnRemoteRequestInner(code, data, reply, option);
}

ErrCode StandbyServiceSubscriberStub::OnRemoteRequestInner(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    switch (code) {
        case (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_DEVICE_IDLE_MODE)): {
            return HandleOnDeviceIdleMode(data);
        }
        case (static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED)): {
            return HandleOnAllowListChanged(data);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_OK;
}

void StandbyServiceSubscriberStub::OnDeviceIdleMode(bool napped, bool sleeped)
{}

void StandbyServiceSubscriberStub::OnAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType,
    bool added)
{}

ErrCode StandbyServiceSubscriberStub::HandleOnDeviceIdleMode(MessageParcel& data)
{
    bool napped {false};
    bool sleeped {false};
    if (!data.ReadBool(napped) || !data.ReadBool(sleeped)) {
        STANDBYSERVICE_LOGW("HandleOnDeviceIdleMode Read parcel failed.");
        return ERR_INVALID_DATA;
    }
    OnDeviceIdleMode(napped, sleeped);
    return ERR_OK;
}

ErrCode StandbyServiceSubscriberStub::HandleOnAllowListChanged(MessageParcel& data)
{
    int32_t uid {0};
    std::string name;
    uint32_t allowType {0};
    bool added {false};
    if (!data.ReadInt32(uid) || !data.ReadString(name) ||
        !data.ReadUint32(allowType) || !data.ReadBool(added)) {
        STANDBYSERVICE_LOGW("HandleOnAllowListChanged Read parcel failed.");
        return ERR_INVALID_DATA;
    }
    OnAllowListChanged(uid, name, allowType, added);
    return ERR_OK;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS