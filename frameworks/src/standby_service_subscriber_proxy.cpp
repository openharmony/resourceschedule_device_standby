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

#include "standby_service_subscriber_proxy.h"

#include <message_parcel.h>

#include "standby_service_errors.h"
#include "standby_service_log.h"
#include "standby_ipc_interface_code.h"

namespace OHOS {
namespace DevStandbyMgr {
StandbyServiceSubscriberProxy::StandbyServiceSubscriberProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IStandbyServiceSubscriber>(impl) {}
StandbyServiceSubscriberProxy::~StandbyServiceSubscriberProxy() {}

void StandbyServiceSubscriberProxy::OnDeviceIdleMode(bool napped, bool sleeping)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        STANDBYSERVICE_LOGW("OnDeviceIdleMode remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(StandbyServiceSubscriberProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("OnDeviceIdleMode write interface token failed.");
        return;
    }

    if (!data.WriteBool(napped) || !data.WriteBool(sleeping)) {
        STANDBYSERVICE_LOGW("OnDeviceIdleMode write parameter failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_DEVICE_IDLE_MODE), data, reply, option);
    if (ret!= ERR_OK) {
        STANDBYSERVICE_LOGE("OnDeviceIdleMode SendRequest failed, error code: %d", ret);
    }
}

void StandbyServiceSubscriberProxy::OnAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType,
    bool added)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        STANDBYSERVICE_LOGW("OnAllowListChanged remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(StandbyServiceSubscriberProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("OnAllowListChanged write interface token failed.");
        return;
    }

    if (!data.WriteInt32(uid) || !data.WriteString(name) ||
        !data.WriteUint32(allowType) || !data.WriteBool(added)) {
        STANDBYSERVICE_LOGW("OnAllowListChanged write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(StandbySubscriberInterfaceCode::ON_ALLOW_LIST_CHANGED), data, reply, option);
    if (ret!= ERR_OK) {
        STANDBYSERVICE_LOGE("OnAllowListChanged SendRequest failed, error code: %d", ret);
    }
}
}  // namespace DevStandbyMgr
}  // namespace OHOS