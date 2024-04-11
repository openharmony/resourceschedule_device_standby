/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "standby_service_stub.h"

#include <ipc_skeleton.h>
#include <string_ex.h>

#include "istandby_ipc_inteface_code.h"
#include "standby_service_subscriber_proxy.h"
#include "standby_service_errors.h"
#include "standby_service_log.h"

namespace OHOS {
namespace DevStandbyMgr {
ErrCode StandbyServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = StandbyServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        STANDBYSERVICE_LOGE("StandbyServiceStub: Local descriptor not match remote.");
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case static_cast<uint32_t>(IStandbyInterfaceCode::SUBSCRIBE_STANDBY_CALLBACK):
            HandleSubscribeStandbyCallback(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::UNSUBSCRIBE_STANDBY_CALLBACK):
            HandleUnsubscribeStandbyCallback(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::APPLY_ALLOW_RESOURCE):
            HandleApplyAllowResource(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::UNAPPLY_ALLOW_RESOURCE):
            HandleUnapplyAllowResource(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::GET_ALLOW_LIST):
            HandleGetAllowList(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::IS_DEVICE_IN_STANDBY):
            HandleIsDeviceInStandby(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_WORK_SCHEDULER_STATUS):
            HandleReportWorkSchedulerStatus(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::GET_RESTRICT_LIST):
            HandleGetRestrictList(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::IS_STRATEGY_ENABLED):
            HandleIsStrategyEnabled(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_DEVICE_STATE_CHANGED):
            HandleReportDeviceStateChanged(data, reply);
            break;
        case static_cast<uint32_t>(IStandbyInterfaceCode::HANDLE_EVENT):
            HandleCommonEvent(data, reply);
            break;
        default:
            return IRemoteStub<IStandbyService>::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleSubscribeStandbyCallback(MessageParcel& data, MessageParcel& reply)
{
    auto subscriber = iface_cast<IStandbyServiceSubscriber>(data.ReadRemoteObject());
    if (!subscriber) {
        STANDBYSERVICE_LOGW("HandleSubscribeStandbyCallback Read callback fail.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    std::string strategyName = data.ReadString();
    STANDBYSERVICE_LOGD("HandleSubscribeStandbyCallback callback name is %{public}s 11111", strategyName.c_str());
    subscriber->SetSubscriberName(strategyName);

    ErrCode result = SubscribeStandbyCallback(subscriber);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleSubscribeStandbyCallback Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleReportWorkSchedulerStatus(MessageParcel& data, MessageParcel& reply)
{
    bool started {false};
    int32_t uid {0};
    std::string bundleName {""};
    if (!data.ReadBool(started) || !data.ReadInt32(uid) || !data.ReadString(bundleName)) {
        STANDBYSERVICE_LOGW("HandleReportWorkSchedulerStatus ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = ReportWorkSchedulerStatus(started, uid, bundleName);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleReportWorkSchedulerStatus Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleGetRestrictList(MessageParcel& data, MessageParcel& reply)
{
    uint32_t restrictType {0};
    uint32_t reasonCode {0};
    if (!data.ReadUint32(restrictType) || !data.ReadUint32(reasonCode)) {
        STANDBYSERVICE_LOGW("HandleGetRestrictList ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    std::vector<AllowInfo> restrictInfoList {};
    ErrCode result = GetRestrictList(restrictType, restrictInfoList, reasonCode);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleGetRestrictList Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!reply.WriteUint32(restrictInfoList.size())) {
        STANDBYSERVICE_LOGW("HandleGetRestrictList Write result size failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    for (auto& info : restrictInfoList) {
        if (!info.Marshalling(reply)) {
            return ERR_STANDBY_PARCELABLE_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleIsStrategyEnabled(MessageParcel& data, MessageParcel& reply)
{
    bool enabled {false};
    std::string strategyName {""};
    if (!data.ReadString(strategyName)) {
        STANDBYSERVICE_LOGW("HandleIsStrategyEnabled ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = IsDeviceInStandby(enabled);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleIsStrategyEnabled Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!reply.WriteBool(enabled)) {
        STANDBYSERVICE_LOGW("HandleIsStrategyEnabled Write enabled failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleUnsubscribeStandbyCallback(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGW("HandleUnsubscribeStandbyCallback Read callback fail.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = UnsubscribeStandbyCallback(iface_cast<IStandbyServiceSubscriber>(subscriber));
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleUnsubscribeStandbyCallback Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleApplyAllowResource(MessageParcel& data, MessageParcel& reply)
{
    auto resourceRequest = ResourceRequest::Unmarshalling(data);
    if (resourceRequest == nullptr) {
        STANDBYSERVICE_LOGW("HandleApplyAllowResource ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = ApplyAllowResource(resourceRequest);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleApplyAllowResource Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleUnapplyAllowResource(MessageParcel& data, MessageParcel& reply)
{
    auto resourceRequest = ResourceRequest::Unmarshalling(data);
    if (resourceRequest == nullptr) {
        STANDBYSERVICE_LOGW("HandleUnapplyAllowResource ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = UnapplyAllowResource(resourceRequest);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleUnapplyAllowResource Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleGetAllowList(MessageParcel& data, MessageParcel& reply)
{
    uint32_t allowType {0};
    uint32_t reasonCode {0};
    if (!data.ReadUint32(allowType) || !data.ReadUint32(reasonCode)) {
        STANDBYSERVICE_LOGW("HandleGetAllowList ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    std::vector<AllowInfo> allowInfoList {};
    ErrCode result = GetAllowList(allowType, allowInfoList, reasonCode);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleGetAllowList Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!reply.WriteUint32(allowInfoList.size())) {
        STANDBYSERVICE_LOGW("HandleGetAllowList Write result size failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    for (auto& info : allowInfoList) {
        if (!info.Marshalling(reply)) {
            return ERR_STANDBY_PARCELABLE_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleIsDeviceInStandby(MessageParcel& data, MessageParcel& reply)
{
    bool isStandby {false};
    ErrCode result = IsDeviceInStandby(isStandby);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleIsDeviceInStandby Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!reply.WriteBool(isStandby)) {
        STANDBYSERVICE_LOGW("HandleIsDeviceInStandby Write isStandby failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleReportDeviceStateChanged(MessageParcel& data, MessageParcel& reply)
{
    int32_t type {0};
    bool enable {false};
    if (!data.ReadInt32(type) || !data.ReadBool(enable)) {
        STANDBYSERVICE_LOGW("HandleReportDeviceStateChanged ReadParcelable failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = ReportDeviceStateChanged(static_cast<DeviceStateType>(type), enable);
    if (!reply.WriteInt32(result)) {
        STANDBYSERVICE_LOGW("HandleReportDeviceStateChanged Write result failed, ErrCode=%{public}d", result);
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode StandbyServiceStub::HandleCommonEvent(MessageParcel& data, MessageParcel& reply)
{
    uint32_t resType = 0;
    if (!data.ReadUint32(resType)) {
        STANDBYSERVICE_LOGW("Failed to read resType");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    int64_t value = 0;
    if (!data.ReadInt64(value)) {
        STANDBYSERVICE_LOGW("Failed to read value");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    std::string sceneInfo = "";
    if (!data.ReadString(sceneInfo)) {
        STANDBYSERVICE_LOGW("Failed to read sceneInfo");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return HandleEvent(resType, value, sceneInfo);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS