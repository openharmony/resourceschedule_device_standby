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

#include "common_event_observer.h"

#include "bundle_constants.h"
#include "common_event_support.h"

#include "standby_service_impl.h"

namespace OHOS {
namespace DevStandbyMgr {
CommonEventObserver::CommonEventObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    const std::shared_ptr<AppExecFwk::EventHandler>& handler)
    : EventFwk::CommonEventSubscriber(subscribeInfo), handler_(handler) {}

bool WEAK_FUNC CommonEventObserver::Subscribe()
{
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this())) {
        STANDBYSERVICE_LOGI("SubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

bool WEAK_FUNC CommonEventObserver::Unsubscribe()
{
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(shared_from_this())) {
        STANDBYSERVICE_LOGI("UnsubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

void CommonEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    AAFwk::Want want = eventData.GetWant();
    std::string action = want.GetAction();
    STANDBYSERVICE_LOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED) {
        std::string bundleName = want.GetElement().GetBundleName();
        int32_t uid = want.GetIntParam(AppExecFwk::Constants::UID, -1);
        handler_->PostTask([uid, bundleName]() {
            StandbyServiceImpl::GetInstance()->RemoveAppAllowRecord(uid, bundleName, true);
            });
    } else {
        handler_->PostTask([]() { StandbyServiceImpl::GetInstance()->ResetTimeObserver(); });
    }
}

std::shared_ptr<CommonEventObserver> CommonEventObserver::CreateCommonEventObserver(
    const std::shared_ptr<AppExecFwk::EventHandler>& handler)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_NITZ_TIMEZONE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_NITZ_TIME_CHANGED);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
    auto observer = std::make_shared<CommonEventObserver>(commonEventSubscribeInfo, handler);
    return observer;
}
}  // namespace DevStandbyMgr
}  // namespace OHOS