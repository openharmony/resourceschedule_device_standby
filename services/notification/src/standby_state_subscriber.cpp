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

#include "standby_state_subscriber.h"

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "want.h"

#include "standby_messsage.h"
#include "standby_service_log.h"
#include "standby_state.h"

namespace OHOS {
namespace DevStandbyMgr {
IMPLEMENT_SINGLE_INSTANCE(StandbyStateSubscriber);

StandbyStateSubscriber::StandbyStateSubscriber()
{
    deathRecipient_ = new (std::nothrow) SubscriberDeathRecipient();
}

StandbyStateSubscriber::~StandbyStateSubscriber() {}

ErrCode StandbyStateSubscriber::AddSubscriber(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    STANDBYSERVICE_LOGD("StandbyStateSubscriber start subscriber");
    if (subscriber == NULL) {
        STANDBYSERVICE_LOGI("subscriber is null");
        return ERR_STANDBY_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        STANDBYSERVICE_LOGE("remote in subscriber is null");
        return ERR_STANDBY_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (deathRecipient_ == nullptr) {
        STANDBYSERVICE_LOGW("create death recipient failed");
        return ERR_STANDBY_INVALID_PARAM;
    }
    auto subscriberIter = FindSubcriberObject(remote);
    if (subscriberIter != subscriberList_.end()) {
        STANDBYSERVICE_LOGE("subscriber has already exist");
        return ERR_STANDBY_OBJECT_EXISTS;
    }

    subscriberList_.emplace_back(subscriber);
    remote->AddDeathRecipient(deathRecipient_);
    STANDBYSERVICE_LOGD(" suscriber standby service callback succeed, list.size() is %{public}d",
        static_cast<int32_t>(subscriberList_.size()));
    return ERR_OK;
}

ErrCode StandbyStateSubscriber::RemoveSubscriber(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        STANDBYSERVICE_LOGE("subscriber is null");
        return ERR_STANDBY_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        STANDBYSERVICE_LOGE("remove subscriber failed, remote in subscriber is null");
        return ERR_STANDBY_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (deathRecipient_ == nullptr) {
        STANDBYSERVICE_LOGW("deathRecipient is null");
        return ERR_STANDBY_OBJECT_EXISTS;
    }
    auto subscriberIter = FindSubcriberObject(remote);
    if (subscriberIter == subscriberList_.end()) {
        STANDBYSERVICE_LOGE("request subscriber is not exists");
        return ERR_STANDBY_OBJECT_EXISTS;
    }
    subscriberList_.erase(subscriberIter);
    remote->RemoveDeathRecipient(deathRecipient_);
    STANDBYSERVICE_LOGD("remove subscriber from standby service subscriber succeed");
    return ERR_OK;
}

void StandbyStateSubscriber::ReportStandbyState(uint32_t curState)
{
    bool napped = curState == StandbyState::NAP;
    bool sleeping = curState == StandbyState::SLEEP;
    STANDBYSERVICE_LOGD("start ReportStandbyState, napping is %{public}d, sleeping is %{public}d", napped, sleeping);
    NotifyIdleModeByCallback(napped, sleeping);
    NotifyIdleModeByCommonEvent(napped, sleeping);
}

void StandbyStateSubscriber::NotifyIdleModeByCallback(bool napped, bool sleeping)
{
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (subscriberList_.empty()) {
        return;
    }
    for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
        (*iter)->OnDeviceIdleMode(napped, sleeping);
    }
    STANDBYSERVICE_LOGD("stop callback subscriber list");
}

void StandbyStateSubscriber::NotifyIdleModeByCommonEvent(bool napped, bool sleeping)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    want.SetParam("napped", napped);
    want.SetParam("sleeping", sleeping);
    EventFwk::CommonEventData commonEventData;
    commonEventData.SetWant(want);
    if (!EventFwk::CommonEventManager::PublishCommonEvent(commonEventData)) {
        STANDBYSERVICE_LOGE("PublishCommonEvent for idle mode finished failed");
    } else {
        STANDBYSERVICE_LOGD("PublishCommonEvent for idle mode finished succeed");
    }
}

void StandbyStateSubscriber::ReportAllowListChanged(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    STANDBYSERVICE_LOGI("start ReportAllowListChanged, uid is %{public}d"\
        ", name is %{public}s, allowType is %{public}d", uid, name.c_str(), allowType);
    NotifyAllowChangedByCallback(uid, name, allowType, added);
    NotifyAllowChangedByCommonEvent(uid, name, allowType, added);
}

void StandbyStateSubscriber::NotifyAllowChangedByCallback(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (subscriberList_.empty()) {
        STANDBYSERVICE_LOGW("Sleep State Subscriber List is empty");
        return;
    }
    for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
        (*iter)->OnAllowListChanged(uid, name, allowType, added);
    }
}

void StandbyStateSubscriber::NotifyAllowChangedByCommonEvent(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_EXEMPTION_LIST_UPDATED);
    want.SetParam("uid", uid);
    want.SetParam("name", name);
    want.SetParam("resourceType", static_cast<int32_t>(allowType));
    want.SetParam("added", added);
    EventFwk::CommonEventData commonEventData;
    commonEventData.SetWant(want);
    if (!EventFwk::CommonEventManager::PublishCommonEvent(commonEventData)) {
        STANDBYSERVICE_LOGE("PublishCommonEvent for exempt list update failed");
    } else {
        STANDBYSERVICE_LOGD("PublishCommonEvent for exempt list update succeed");
    }
}

void StandbyStateSubscriber::HandleSubscriberDeath(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        STANDBYSERVICE_LOGE("suscriber death, remote in suscriber is null");
        return;
    }
    sptr<IRemoteObject> proxy = remote.promote();
    if (!proxy) {
        STANDBYSERVICE_LOGE("get remote proxy failed");
        return;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    auto subscriberIter = FindSubcriberObject(proxy);
    if (subscriberIter == subscriberList_.end()) {
        STANDBYSERVICE_LOGI("suscriber death, remote in suscriber not found");
        return;
    }
    subscriberList_.erase(subscriberIter);
    STANDBYSERVICE_LOGD("suscriber death, remove it from list");
}

void StandbyStateSubscriber::ShellDump(const std::vector<std::string>& argsInStr, std::string& result)
{
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (subscriberList_.empty()) {
        result += "subscriber observer record is empty\n";
        return;
    }
    std::stringstream stream;
    for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
        stream << "\tobserverName: " << (*iter)->GetSubscriberName() << "\n";
        result += stream.str();
        stream.clear();
    }
}

std::list<sptr<IStandbyServiceSubscriber>>::iterator StandbyStateSubscriber::FindSubcriberObject(
    sptr<IRemoteObject>& proxy)
{
    auto findSuscriber = [&proxy](const auto& subscriber) {
        return subscriber->AsObject() == proxy;
    };
    return std::find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
}

SubscriberDeathRecipient::SubscriberDeathRecipient() {}

SubscriberDeathRecipient::~SubscriberDeathRecipient() {}

void SubscriberDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    StandbyStateSubscriber::GetInstance()->HandleSubscriberDeath(remote);
}
}  // namespace DevStandbyMgr
}  // namespace OHOS