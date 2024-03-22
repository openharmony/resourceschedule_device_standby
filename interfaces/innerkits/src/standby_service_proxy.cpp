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

#include "standby_service_proxy.h"

#include <message_parcel.h>

#include "standby_service_errors.h"
#include "standby_service_log.h"
#include "istandby_ipc_inteface_code.h"

namespace OHOS {
namespace DevStandbyMgr {
StandbyServiceProxy::StandbyServiceProxy(const sptr<IRemoteObject>& impl)
    :IRemoteProxy<IStandbyService>(impl) {}
StandbyServiceProxy::~StandbyServiceProxy() {}

ErrCode StandbyServiceProxy::SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent subscriber is null");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent write subscriber failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteString(subscriber->GetSubscriberName())) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent write SubscriberName failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(
        static_cast<uint32_t>(IStandbyInterfaceCode::SUBSCRIBE_STANDBY_CALLBACK), option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent fail: read result failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("SubscribeSleepStateEvent failed");
    }
    return result;
}

ErrCode StandbyServiceProxy::UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent subscriber is null");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent write subscriber failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::UNSUBSCRIBE_STANDBY_CALLBACK),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent fail: read result failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("UnsubscribeSleepStateEvent failed");
        return result;
    }
    return result;
}

ErrCode StandbyServiceProxy::ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("ApplyAllowResource write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!resourceRequest->Marshalling(data)) {
        STANDBYSERVICE_LOGW("ApplyAllowResource write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::APPLY_ALLOW_RESOURCE),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ApplyAllowResource fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("ApplyAllowResource fail: read result failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ApplyAllowResource failed");
        return result;
    }
    return result;
}

ErrCode StandbyServiceProxy::UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("RemoveAllowList write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!resourceRequest->Marshalling(data)) {
        STANDBYSERVICE_LOGW("RemoveAllowList write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::UNAPPLY_ALLOW_RESOURCE),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("RemoveAllowList fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("RemoveAllowList fail: read result failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("RemoveAllowList failed");
        return result;
    }
    return result;
}

ErrCode StandbyServiceProxy::GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoList,
    uint32_t reasonCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("GetAllowList write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteUint32(allowType) || !data.WriteUint32(reasonCode)) {
        STANDBYSERVICE_LOGW("GetAllowList write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::GET_ALLOW_LIST),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("GetAllowList fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("GetAllowList fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("GetAllowList failed");
        return result;
    }
    uint32_t infoSize = reply.ReadUint32();
    for (uint32_t i = 0; i < infoSize; i++) {
        auto info = AllowInfo::Unmarshalling(reply);
        if (info == nullptr) {
            STANDBYSERVICE_LOGW("GetAllowList Read Parcelable infos failed.");
            return ERR_STANDBY_PARCELABLE_FAILED;
        }
        allowInfoList.emplace_back(*info);
    }

    return result;
}

ErrCode StandbyServiceProxy::IsDeviceInStandby(bool& isStandby)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::IS_DEVICE_IN_STANDBY),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby failed");
        return result;
    }
    if (!reply.ReadBool(isStandby)) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode StandbyServiceProxy::ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    if (!data.WriteBool(started) || !data.WriteInt32(uid) || !data.WriteString(bundleName)) {
        STANDBYSERVICE_LOGW("ReportWorkSchedulerStatus write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_WORK_SCHEDULER_STATUS),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ReportWorkSchedulerStatus fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("ReportWorkSchedulerStatus fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ReportWorkSchedulerStatus failed");
        return result;
    }
    return result;
}

ErrCode StandbyServiceProxy::GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
    uint32_t reasonCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("GetRestrictList write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteUint32(restrictType) || !data.WriteUint32(reasonCode)) {
        STANDBYSERVICE_LOGW("GetRestrictList write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::GET_RESTRICT_LIST),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("GetRestrictList fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("GetRestrictList fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("GetRestrictList failed");
        return result;
    }
    uint32_t infoSize = reply.ReadUint32();
    for (uint32_t i = 0; i < infoSize; i++) {
        auto info = AllowInfo::Unmarshalling(reply);
        if (info == nullptr) {
            STANDBYSERVICE_LOGW("GetRestrictList Read Parcelable infos failed.");
            return ERR_STANDBY_PARCELABLE_FAILED;
        }
        restrictInfoList.emplace_back(*info);
    }

    return result;
}

ErrCode StandbyServiceProxy::IsStrategyEnabled(const std::string& strategyName, bool& enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (!data.WriteString(strategyName)) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::IS_STRATEGY_ENABLED),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled failed");
        return result;
    }
    if (!reply.ReadBool(enabled)) {
        STANDBYSERVICE_LOGW("IsStrategyEnabled fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode StandbyServiceProxy::ReportDeviceStateChanged(DeviceStateType type, bool enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("IsDeviceInStandby write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(static_cast<int32_t>(type)) || !data.WriteBool(enabled)) {
        STANDBYSERVICE_LOGW("ReportDeviceStateChanged write parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::REPORT_DEVICE_STATE_CHANGED),
        option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ReportDeviceStateChanged fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("ReportDeviceStateChanged fail: read result failed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("ReportDeviceStateChanged failed");
        return result;
    }
    return result;
}

ErrCode StandbyServiceProxy::InnerTransact(uint32_t code, MessageOption& flags,
    MessageParcel& data, MessageParcel& reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        STANDBYSERVICE_LOGE("InnerTransact get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int32_t err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            STANDBYSERVICE_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            STANDBYSERVICE_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_STANDBY_TRANSACT_FAILED;
        }
    }
}

ErrCode StandbyServiceProxy::HandleEvent(const uint32_t resType, const int64_t value, const std::string &sceneInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(StandbyServiceProxy::GetDescriptor())) {
        STANDBYSERVICE_LOGW("HandleEvent write descriptor failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }

    if (!data.WriteUint32(resType) || !data.WriteInt64(value) || !data.WriteString(sceneInfo)) {
        STANDBYSERVICE_LOGW("ReportDeviceStateChanged wirte parameter failed");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(IStandbyInterfaceCode::HANDLE_EVENT),
                                   option, data, reply);
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("HandleEvent fail: transact ErrCode=%{public}d", result);
        return ERR_STANDBY_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        STANDBYSERVICE_LOGW("HandleEvent fail: read result dailed.");
        return ERR_STANDBY_PARCELABLE_FAILED;
    }
    if (result != ERR_OK) {
        STANDBYSERVICE_LOGW("HandleEvent failed");
        return result;
    }
    return result;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS