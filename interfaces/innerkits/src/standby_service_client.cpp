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

#include "standby_service_client.h"

#include <message_parcel.h>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "standby_service_errors.h"
#include "standby_service_log.h"
#include "standby_service_proxy.h"

namespace OHOS {
namespace DevStandbyMgr {
StandbyServiceClient::StandbyServiceClient() {}

StandbyServiceClient::~StandbyServiceClient() {}

StandbyServiceClient& StandbyServiceClient::GetInstance()
{
    static StandbyServiceClient StandbyServiceClient;
    return StandbyServiceClient;
}

ErrCode StandbyServiceClient::SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGE("subscriber is nullptr");
        return ERR_STANDBY_INVALID_PARAM;
    }
    std::string subscriberName = subscriber->GetSubscriberName();
    std::string moduleName = subscriber->GetModuleName();
    return standbyServiceProxy_->SubscribeStandbyCallback(subscriber, subscriberName, moduleName);
}

ErrCode StandbyServiceClient::UnsubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGE("subscriber is nullptr");
        return ERR_STANDBY_INVALID_PARAM;
    }
    return standbyServiceProxy_->UnsubscribeStandbyCallback(subscriber);
}

ErrCode StandbyServiceClient::ApplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (resourceRequest == nullptr) {
        STANDBYSERVICE_LOGE("resource request is nullptr");
        return ERR_STANDBY_INVALID_PARAM;
    }
    ResourceRequest& request = *resourceRequest.GetRefPtr();
    return standbyServiceProxy_->ApplyAllowResource(request);
}

ErrCode StandbyServiceClient::UnapplyAllowResource(const sptr<ResourceRequest>& resourceRequest)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (resourceRequest == nullptr) {
        STANDBYSERVICE_LOGE("resource request is nullptr");
        return ERR_STANDBY_INVALID_PARAM;
    }
    ResourceRequest& request = *resourceRequest.GetRefPtr();
    return standbyServiceProxy_->UnapplyAllowResource(request);
}

ErrCode StandbyServiceClient::GetAllowList(uint32_t allowType, std::vector<AllowInfo>& allowInfoArray,
    uint32_t reasonCode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (!allowInfoArray.empty()) {
        STANDBYSERVICE_LOGW("allow info array is not empty");
        allowInfoArray.clear();
    }
    return standbyServiceProxy_->GetAllowList(allowType, allowInfoArray, reasonCode);
}

ErrCode StandbyServiceClient::IsDeviceInStandby(bool& isStandby)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->IsDeviceInStandby(isStandby);
}

ErrCode StandbyServiceClient::SetNatInterval(uint32_t& type, bool& enable, uint32_t& interval)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->SetNatInterval(type, enable, interval);
}

ErrCode StandbyServiceClient::HandleEvent(const std::shared_ptr<ResourceSchedule::ResData> &resData)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string sceneInfo = resData->payload.dump();
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->HandleEvent(resData->resType, resData->value, sceneInfo);
}

ErrCode StandbyServiceClient::ReportWorkSchedulerStatus(bool started, int32_t uid, const std::string& bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->ReportWorkSchedulerStatus(started, uid, bundleName);
}

ErrCode StandbyServiceClient::GetRestrictList(uint32_t restrictType, std::vector<AllowInfo>& restrictInfoList,
    uint32_t reasonCode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    if (!restrictInfoList.empty()) {
        STANDBYSERVICE_LOGW("restrict info array is not empty");
        restrictInfoList.clear();
    }
    return standbyServiceProxy_->GetRestrictList(restrictType, restrictInfoList, reasonCode);
}

ErrCode StandbyServiceClient::IsStrategyEnabled(const std::string& strategyName, bool& isEnabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->IsStrategyEnabled(strategyName, isEnabled);
}

ErrCode StandbyServiceClient::ReportPowerOverused(const std::string &module, uint32_t level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    STANDBYSERVICE_LOGD("[PowerOverused] StandbyClient: power overused, module name: %{public}s, "
        "level: %{public}u.", module.c_str(), level);

    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->ReportPowerOverused(module, level);
}

ErrCode StandbyServiceClient::DelayHeartBeat(int64_t timestamp)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    return standbyServiceProxy_->DelayHeartBeat(timestamp);
}

ErrCode StandbyServiceClient::ReportDeviceStateChanged(DeviceStateType type, bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    STANDBYSERVICE_LOGI("device state changed, state type: %{public}d, enabled: %{public}d",
        static_cast<int32_t>(type), enabled);
    if (!GetStandbyServiceProxy()) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return ERR_STANDBY_SERVICE_NOT_CONNECTED;
    }
    int32_t transType = static_cast<int32_t>(type);
    return standbyServiceProxy_->ReportDeviceStateChanged(transType, enabled);
}

bool StandbyServiceClient::GetStandbyServiceProxy()
{
    if (standbyServiceProxy_ != nullptr) {
        return true;
    }
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        STANDBYSERVICE_LOGE("get standby service proxy failed");
        return false;
    }

    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
    if (remoteObject == nullptr) {
        STANDBYSERVICE_LOGE("get standby service system ability failed");
        return false;
    }

    standbyServiceProxy_ = iface_cast<IStandbyService>(remoteObject);
    if ((standbyServiceProxy_ == nullptr) || (standbyServiceProxy_->AsObject() == nullptr)) {
        STANDBYSERVICE_LOGE("standby service proxy iface_cast from remote Onject failed");
        return false;
    }

    deathRecipient_ = new (std::nothrow) StandbyServiceDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        return false;
    }

    standbyServiceProxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    return true;
}

void StandbyServiceClient::ResetStandbyServiceClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((standbyServiceProxy_ != nullptr)&& (standbyServiceProxy_->AsObject() != nullptr)) {
        standbyServiceProxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    standbyServiceProxy_ = nullptr;
}

StandbyServiceClient::StandbyServiceDeathRecipient::StandbyServiceDeathRecipient(
    StandbyServiceClient& standbyServiceClient)
    : standbyServiceClient_(standbyServiceClient) {}

StandbyServiceClient::StandbyServiceDeathRecipient::~StandbyServiceDeathRecipient() {}

void StandbyServiceClient::StandbyServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    standbyServiceClient_.ResetStandbyServiceClient();
}
}  // namespace DevStandbyMgr
}  // namespace OHOS
